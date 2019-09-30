#include "bufferededitor.h"

#include <QFileDevice>

#include <QDebug>

BufferedEditor::BufferedEditor(QFileDevice *device, QObject *parent)
	: QObject(parent)
	, m_device(device)
	, m_sectionIndex(-1)
	, m_localPosition(sectionSize)
	, m_absolutePosition(0)
	, m_section(m_sections.end())
	, m_size(device->size())
	, m_modificationCount(0)
{
}

QString BufferedEditor::errorString() const
{
	return m_device->errorString();
}

bool BufferedEditor::seek(qint64 position)
{
	int sectionIndex = int(position / sectionSize);
	auto iterator = m_sections.find(sectionIndex);
	if (iterator == m_sections.end()) {
		iterator = loadSection(sectionIndex);
		if (iterator == m_sections.end())
			return false;
	}

	m_section = iterator;
	m_sectionIndex = sectionIndex;
	m_localPosition = position % sectionSize;
	m_absolutePosition = position;
	return true;
}

qint64 BufferedEditor::position() const
{
	return m_absolutePosition;
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
	return m_absolutePosition == m_device->size();
}

char BufferedEditor::getByte()
{
	if (m_localPosition == sectionSize) {
		m_localPosition = 0;
		m_section = getSection(++m_sectionIndex);
		Q_ASSERT(m_section != m_sections.end());
	}
	++m_absolutePosition;
	return m_section->data[m_localPosition++];
}

void BufferedEditor::putByte(char byte)
{
	if (m_localPosition == sectionSize) {
		m_localPosition = 0;
		m_section = getSection(++m_sectionIndex);
		Q_ASSERT(m_section != m_sections.end());
	}
	Section &section = *m_section;

	if (m_localPosition == section.length) {
		++section.length;
		++m_size;
		m_modifications.append(Insertion(byte, quint16(m_localPosition), m_sectionIndex));
	} else {
		m_modifications.append(Replacement(section.data[m_localPosition], byte, quint16(m_localPosition), m_sectionIndex));
	}

	section.data[m_localPosition++] = byte;
	++section.modificationCount;

	++m_modificationCount;
	++m_absolutePosition;

	if (m_modifications.size() == 1) {
		Q_ASSERT(canUndo());
		emit canUndoChanged(true);
	}
}

bool BufferedEditor::writeChanges()
{
	for (auto it = m_sections.begin(); it != m_sections.end(); ++it) {
		Section &section = *it;
		if (section.modificationCount) {
			qDebug() << "Writing section" << it.key();
			qDebug() << section.length;
			if (!m_device->seek(it.key()))
				return false;
			if (!m_device->write(section.data, section.length))
				return false;
			qDebug() << "Wrote section" << it.key();
			m_modificationCount -= section.modificationCount;
			section.modificationCount = 0;
		}
	}
	Q_ASSERT(m_modificationCount == 0);
	// TODO: If resizing fails, isModified() returns false, should be true
	if (m_device->size() != m_size)
		if (!m_device->resize(m_size))
			return false;
	return true;
}

bool BufferedEditor::isModified() const
{
	return m_modificationCount != 0;
}

bool BufferedEditor::canUndo() const
{
	return !m_modifications.isEmpty();
}

void BufferedEditor::undo()
{
	if (!canUndo())
		return;

	auto var = m_modifications.last();
	if (std::holds_alternative<Replacement>(var)) {
		Replacement replacement = std::get<Replacement>(var);
		auto section = m_sections.find(replacement.sectionIndex);
		section->data[replacement.localPosition] = replacement.before;
		--section->modificationCount;
	} else {
		Insertion insertion = std::get<Insertion>(var);
		qDebug() << "Undo: Insertion in" << insertion.sectionIndex;
		auto section = m_sections.find(insertion.sectionIndex);
		--section->length;
		--section->modificationCount;
		--m_size;
		if (section->length == 0)
			m_sections.erase(section);
	}

	--m_modificationCount;
	m_modifications.removeLast();

	if (m_modifications.isEmpty()) {
		Q_ASSERT(!canUndo());
		emit canUndoChanged(false);
	}
}


QMap<int, BufferedEditor::Section>::iterator BufferedEditor::loadSection(int sectionIndex)
{
	Q_ASSERT(!m_sections.contains(sectionIndex));
	Section section;
	qint64 startByte = sectionIndex * sectionSize;
	if (!m_device->seek(startByte)) {
		// TODO
		qDebug() << "Failed to read from device: " << m_device->errorString();
		return m_sections.end();
	}

	section.length = int(m_device->read(section.data, sectionSize));
	return m_sections.insert(sectionIndex, std::move(section));
}

QMap<int, BufferedEditor::Section>::iterator BufferedEditor::getSection(int sectionIndex)
{
	auto iterator = m_sections.find(sectionIndex);
	if (iterator == m_sections.end())
		iterator = loadSection(sectionIndex);
	return iterator;
}
