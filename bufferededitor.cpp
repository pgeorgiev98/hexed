#include "bufferededitor.h"

#include <QIODevice>

#include <QDebug>

BufferedEditor::BufferedEditor(QIODevice *device)
	: m_device(device)
	, m_sectionIndex(-1)
	, m_localPosition(sectionSize)
	, m_absolutePosition(0)
	, m_section(m_sections.end())
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
	return m_device->size();
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
		auto iterator = m_sections.find(m_sectionIndex);
		if (iterator == m_sections.end()) {
			iterator = loadSection(m_sectionIndex);
		}
		Q_ASSERT(m_section != m_sections.end());
		++m_sectionIndex;
	}
	++m_absolutePosition;
	return m_section->data[m_localPosition++];
}


QMap<int, BufferedEditor::Section>::iterator BufferedEditor::loadSection(int sectionIndex)
{
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
