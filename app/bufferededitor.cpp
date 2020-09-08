#include "bufferededitor.h"

#include <QFileDevice>

#include <QDebug>

BufferedEditor::BufferedEditor(QFileDevice *device, QObject *parent)
	: QObject(parent)
	, m_device(device)
	, m_sectionIndex(-1)
	, m_sectionLocalPosition(0)
	, m_position(0)
	, m_size(device->size())
	, m_currentModificationIndex(0)
	, m_modificationCount(0)
{
}

QString BufferedEditor::errorString() const
{
	return m_device->errorString();
}

bool BufferedEditor::seek(qint64 position)
{
	if (position == m_size) {
		m_sectionIndex = -1;
		m_sectionLocalPosition = 0;
		m_position = m_size;
		return true;
	}

	m_sectionIndex = getSectionIndex(position);
	if (m_sectionIndex == -1)
		return false;

	m_position = position;

	const Section &s = m_sections[m_sectionIndex];
	m_sectionLocalPosition = s.bytePosition(position);

	return true;
}

qint64 BufferedEditor::position() const
{
	return m_position;
}

qint64 BufferedEditor::size() const
{
	return m_size;
}

bool BufferedEditor::isEmpty() const
{
	return m_device->size() == 0;
}

bool BufferedEditor::atEnd() const
{
	return m_position == m_size;
}

void BufferedEditor::moveForward()
{
	if (atEnd())
		return;

	const Section &s = m_sections[m_sectionIndex];
	for (;;) {
		++m_sectionLocalPosition;
		if (m_sectionLocalPosition == s.data.size()) {
			seek(m_position + 1); // TODO: optimize this somehow
			return;
		}
		if (s.data[m_sectionLocalPosition].current)
			break;
	}
	++m_position;
}

BufferedEditor::Byte BufferedEditor::getByte()
{
	auto byte = m_sections[m_sectionIndex].data[m_sectionLocalPosition];
	Q_ASSERT(byte.current);
	moveForward();
	return byte;
}

void BufferedEditor::replaceByte(char byte)
{
	userDoModification(Modification(Modification::Type::Replace, byte, m_sectionIndex, m_sectionLocalPosition));
}

void BufferedEditor::insertByte(char byte)
{
	userDoModification(Modification(Modification::Type::Insert, byte, m_sectionIndex, m_sectionLocalPosition));
}

void BufferedEditor::deleteByte()
{
	userDoModification(Modification(Modification::Type::Delete, char(0), m_sectionIndex, m_sectionLocalPosition));
}

bool BufferedEditor::writeChanges()
{
	// Needed for the dummy section
	qint64 oldFileSize = m_device->size();

	// Increase the file size if needed
	if (m_device->size() < m_size)
		if (!m_device->resize(m_size))
			return false;

	// The UnchangedSection objects are sections of the file that are
	// not modified or loaded into memory, but have to be *moved*
	// to a different position in the file because of insertions
	// or deletions that have happened in some loaded sections

	// Those sections will be limited to a size of `sectionSize`
	// to prevent high memory usage during the reallocation

	struct UnchangedSection
	{
		qint64 oldPosition, newPosition;
		int length;
	};

	QVector<UnchangedSection> unchangedSections;

	// Make a list of the UnchangedSections
	Section dummySection(oldFileSize, m_size); // Dummy end section
	Section &firstSection = m_sections.isEmpty() ? dummySection : m_sections.first();
	qint64 savedPosition = firstSection.savedPosition, currentPosition = firstSection.currentPosition;
	for (int i = 0; i <= m_sections.size(); ++i) {
		const Section &section = i < m_sections.size() ? m_sections[i] : dummySection;
		Q_ASSERT(savedPosition <= section.savedPosition);

		if (savedPosition != section.savedPosition) {
			// The length of the unchanged section shouldn't be changed
			Q_ASSERT(section.savedPosition - savedPosition == section.currentPosition - currentPosition);

			qint64 length = section.savedPosition - savedPosition;
			if (savedPosition != currentPosition) {
				qint64 index = 0;
				while (index < length) {
					UnchangedSection s;
					s.oldPosition = savedPosition + index;
					s.newPosition = currentPosition + index;
					s.length = int(qMin(qint64(sectionSize), length - index));
					unchangedSections.append(s);
					index += s.length;
				}
			}

			savedPosition += length;
			currentPosition += length;
		}
		savedPosition += section.savedLength();
		currentPosition += section.currentLength();
	}

	qDebug() << unchangedSections.size() << "unchanged sections have to be moved";

	// Sort the sections in the order that they have to be written to the disk
	QVector<UnchangedSection> sortedUnchangedSections;
	while (!unchangedSections.isEmpty()) {
		// Find a section s1 that won't be written over another section
		int attemptCount = 0;
		int j = 0;
		UnchangedSection s1 = unchangedSections[j];
		for (int i = 0; i < unchangedSections.size(); ++i) {
			if (i != j) {
				UnchangedSection s2 = unchangedSections[i];
				// If s1 will be written over s2
				if (s1.newPosition < s2.oldPosition + s2.length &&
						s1.newPosition + s1.length > s2.oldPosition) {
					// Restart the search
					s1 = s2;
					j = i;
					i = 0;
					++attemptCount;
					Q_ASSERT(attemptCount <= unchangedSections.size());
					if (attemptCount > unchangedSections.size())
						return false;
				}
			}
		}
		unchangedSections.removeAt(j);
		sortedUnchangedSections.append(s1);
	}

	// Move those sections
	for (int i = 0; i < sortedUnchangedSections.size(); ++i) {
		UnchangedSection s = sortedUnchangedSections[i];
		qDebug("Moving section %d/%d with a length of %d from %lld to %lld",
			   i, sortedUnchangedSections.size() - 1,
			   s.length, s.oldPosition, s.newPosition);

		QVector<char> buffer(int(s.length));

		if (!m_device->seek(s.oldPosition)) {
			qCritical() << "BufferedEditor: Failed to seek in file:" << m_device->errorString();
			return false;
		}
		qint64 bytesRead = m_device->read(buffer.data(), s.length);
		if (bytesRead == -1) {
			qCritical() << "Failed to read from file:" << m_device->errorString();
			return false;
		}
		Q_ASSERT(bytesRead == s.length);

		if (!m_device->seek(s.newPosition)) {
			qCritical() << "BufferedEditor: Failed to seek in file:" << m_device->errorString();
			return false;
		}
		qint64 bytesWritten = m_device->write(buffer.data(), s.length);
		if (bytesWritten == -1) {
			qCritical() << "Failed to write to file:" << m_device->errorString();
			return false;
		}
		Q_ASSERT(bytesWritten == s.length);
	}

	// Write the modified sections
	for (int i = 0; i < m_sections.size(); ++i) {
		Section &s = m_sections[i];
		if (s.isModified() || s.savedPosition != s.currentPosition) {
			qDebug("Writing section %d (%d)", i, m_sections.size());
			QVector<char> buffer;
			for (Byte b : s.data)
				if (b.current)
					buffer.append(*b.current);

			if (!m_device->seek(s.currentPosition)) {
				qCritical() << "BufferedEditor: Failed to seek in file:" << m_device->errorString();
				return false;
			}

			qint64 bytesWritten = m_device->write(buffer.data(), buffer.size());
			if (bytesWritten == -1) {
				qCritical() << "Failed to write to file:" << m_device->errorString();
				return false;
			}
			Q_ASSERT(bytesWritten == buffer.size());

			s.modificationCount = 0;
			s.savedPosition = s.currentPosition;
			for (Byte &b : s.data)
				b.saved = b.current;
		}
	}

	if (m_device->size() > m_size)
		if (!m_device->resize(m_size))
			return false;

	m_device->flush();

	m_modificationCount = 0;

	return true;
}

bool BufferedEditor::isModified() const
{
	return m_modificationCount != 0;
}

bool BufferedEditor::canUndo() const
{
	return m_currentModificationIndex > 0;
}

bool BufferedEditor::canRedo() const
{
	return m_currentModificationIndex < m_modifications.size();
}

void BufferedEditor::undo()
{
	if (!canUndo())
		return;

	undoModification(m_modifications[m_currentModificationIndex - 1]);

	--m_modificationCount;
	--m_currentModificationIndex;

	emit canRedoChanged(true);

	if (!canUndo())
		emit canUndoChanged(false);
}

void BufferedEditor::redo()
{
	if (!canRedo())
		return;

	doModification(m_modifications[m_currentModificationIndex]);

	++m_modificationCount;
	++m_currentModificationIndex;

	emit canUndoChanged(true);

	if (!canRedo())
		emit canRedoChanged(false);
}


int BufferedEditor::getSectionIndex(qint64 position)
{
	// The index of the section
	int index = -1;
	// The index of the next/previous section if such is loaded and the section was not found
	int nextIndex = -1, prevIndex = -1;

	// Find the needed section
	// TODO: Maybe use binary search or at least optimize for sequential access
	for (int i = 0; i < m_sections.size(); ++i) {
		const Section &section = m_sections[i];
		if (section.currentPosition > position) {
			nextIndex = i;
			break;
		}
		if (position >= section.currentPosition &&
				position < section.currentPosition + section.currentLength()) {
			index = i;
			break;
		}
		prevIndex = i;
	}

	if (index == -1) {
		// Section is not loaded in memory
		qDebug() << "BufferedEditor: Loading section for byte" << position;

		// The location of `position` in the actual, unedited file
		qint64 realPosition = position;
		qint64 prevSectionSavedEnd = 0;
		if (prevIndex != -1) {
			prevSectionSavedEnd = m_sections[prevIndex].savedPosition + m_sections[prevIndex].savedLength();
			qint64 prevSectionCurrentEnd = m_sections[prevIndex].currentPosition + m_sections[prevIndex].currentLength();
			realPosition = prevSectionSavedEnd + position - prevSectionCurrentEnd;
		}

		// The maximum allowed 'end' of the section that is to be loaded
		qint64 newSectionMaxEnd;
		if (nextIndex != -1) {
			// Don't overlap with the next section
			newSectionMaxEnd = m_sections[nextIndex].savedPosition;
		} else {
			// Don't try to read after the end of the file
			newSectionMaxEnd = m_device->size();
		}

		// Calculate the new section start/end positions
		qint64 newSectionStart = realPosition;
		qint64 newSectionEnd = newSectionMaxEnd;
		if (newSectionMaxEnd - realPosition < sectionSize)
			newSectionStart = qMax(newSectionMaxEnd - sectionSize, prevSectionSavedEnd);
		else
			newSectionEnd = newSectionStart + sectionSize;
		int newSectionLength = int(newSectionEnd - newSectionStart);

		// Create the section object
		Section section(newSectionStart, newSectionStart + position - realPosition);
		section.data.resize(newSectionLength);

		// Seek the file to the required position
		if (!m_device->seek(newSectionStart)) {
			qCritical() << "BufferedEditor: Failed to seek in file:" << m_device->errorString();
			return -1;
		}

		// Read the data from the file
		QVector<char> buffer(newSectionLength);
		qint64 bytesRead = m_device->read(buffer.data(), newSectionLength);
		if (bytesRead == -1) {
			qCritical() << "BufferedEditor: Failed to write to file:" << m_device->errorString();
			return -1;
		}
		Q_ASSERT(bytesRead == newSectionLength);
		for (int i = 0; i < newSectionLength; ++i) {
			char b = buffer[i];
			section.data[i] = Byte(b, b);
		}

		// Add the new section to the list of loaded sections
		index = nextIndex == -1 ? m_sections.size() : nextIndex;
		m_sections.insert(index, std::move(section));

		// Update the undo events section indices
		for (Modification &m : m_modifications)
			if (m.sectionIndex >= index)
				++m.sectionIndex;
	}

	return index;
}

void BufferedEditor::doModification(Modification &modification)
{
	qint64 oldSize = m_size;
	Section &section = m_sections[modification.sectionIndex];

	switch (modification.type) {
	case Modification::Type::Replace:
	{
		Byte &byte = section.data[modification.byteIndex];
		char oldByte = *byte.current;
		byte.current = modification.byte;
		modification.byte = oldByte;
		break;
	}

	case Modification::Type::Insert:
	{
		Byte byte(std::optional<char>(), modification.byte);
		section.data.insert(modification.byteIndex, byte);
		++m_size;
		updateSectionsPosition(modification.sectionIndex + 1);
		break;
	}

	case Modification::Type::Delete:
	{
		auto &byte = section.data[modification.byteIndex].current;
		modification.byte = *byte;
		byte.reset();
		--m_size;
		updateSectionsPosition(modification.sectionIndex + 1);
		break;
	}
	}

	if (m_size != oldSize)
		emit sizeChanged(m_size);

	++section.modificationCount;
	++m_modificationCount;
}

void BufferedEditor::undoModification(Modification &modification)
{
	qint64 oldSize = m_size;
	Section &section = m_sections[modification.sectionIndex];

	switch (modification.type) {
	case Modification::Type::Replace:
	{
		Byte &byte = section.data[modification.byteIndex];
		char oldByte = *byte.current;
		byte.current = modification.byte;
		modification.byte = oldByte;
		break;
	}

	case Modification::Type::Insert:
	{
		Q_ASSERT(!section.data[modification.byteIndex].saved);
		section.data.removeAt(modification.byteIndex);
		--m_size;
		updateSectionsPosition(modification.sectionIndex + 1);
		break;
	}

	case Modification::Type::Delete:
	{
		std::optional<char> &byte = section.data[modification.byteIndex].current;
		Q_ASSERT(!byte);
		byte = modification.byte;
		++m_size;
		updateSectionsPosition(modification.sectionIndex + 1);
		break;
	}
	}

	--m_modificationCount;
	if (m_size != oldSize)
		emit sizeChanged(m_size);
}

void BufferedEditor::userDoModification(Modification m)
{
	bool couldRedo = canRedo();
	if (couldRedo) {
		m_modifications.remove(m_currentModificationIndex, m_modifications.size() - m_currentModificationIndex);
		Q_ASSERT(!canRedo());
	}

	doModification(m);
	m_modifications.append(m);
	m_currentModificationIndex = m_modifications.size();

	Q_ASSERT(canUndo());
	emit canUndoChanged(true);

	if (couldRedo)
		emit canRedoChanged(false);
}

void BufferedEditor::updateSectionsPosition(int firstSectionIndex)
{
	qint64 savedPosition;
	qint64 currentPosition;
	if (firstSectionIndex > 0) {
		Section &s = m_sections[firstSectionIndex - 1];
		savedPosition = s.savedPosition;
		currentPosition = s.currentPosition;
		savedPosition += s.savedLength();
		currentPosition += s.currentLength();
	} else {
		savedPosition = 0;
		currentPosition = 0;
	}

	for (int i = firstSectionIndex; i < m_sections.size(); ++i) {
		Section &s = m_sections[i];
		qint64 offset = s.savedPosition - savedPosition;
		savedPosition += offset;
		currentPosition += offset;
		s.currentPosition = currentPosition;
		savedPosition += s.savedLength();
		currentPosition += s.currentLength();
	}
}
