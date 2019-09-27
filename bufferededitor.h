#ifndef BUFFEREDEDITOR_H
#define BUFFEREDEDITOR_H

#include <QMap>

class QIODevice;

class BufferedEditor
{
public:
	BufferedEditor(QIODevice *device);
	QString errorString() const;
	bool seek(qint64 position);
	qint64 position() const;
	qint64 size() const;
	bool isEmpty() const;
	bool atEnd() const;
	char getByte();

private:
	static const int sectionSize = 64;

	struct Section
	{
		char data[sectionSize];
		int length;

		Section() : length(0) {}
	};

	QIODevice *m_device;
	QMap<int, Section> m_sections;
	int m_sectionIndex;
	int m_localPosition;
	qint64 m_absolutePosition;
	QMap<int, Section>::iterator m_section;

	QMap<int, Section>::iterator loadSection(int sectionIndex);
};

#endif // BUFFEREDEDITOR_H
