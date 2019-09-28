#include "bufferededitor.h"

#include <QIODevice>

#include <QDebug>

BufferedEditor::BufferedEditor(QIODevice *device)
	: m_device(device)
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
	++m_absolutePosition;
	Section &section = *m_section;
	Modification modification(section.data[m_localPosition],
							  byte,
							  m_sectionIndex);
	m_modifications.append(modification);

	if (m_localPosition == section.length) {
		++section.length;
		++m_size;
	}
	section.data[m_localPosition++] = byte;
	++section.modificationCount;

	++m_modificationCount;
}

bool BufferedEditor::writeChanges()
{
	for (auto it = m_sections.begin(); it != m_sections.end(); ++it) {
		const Section &section = *it;
		if (section.modificationCount) {
			if (!m_device->seek(it.key()))
				return false;
			if (!m_device->write(section.data, section.length))
				return false;
		}
	}
	return true;
}

bool BufferedEditor::isModified() const
{
	return m_modificationCount > 0;
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
