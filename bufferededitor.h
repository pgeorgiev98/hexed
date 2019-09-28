#ifndef BUFFEREDEDITOR_H
#define BUFFEREDEDITOR_H

#include <QMap>
#include <QVector>

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
	void putByte(char byte);
	bool writeChanges();
	bool isModified() const;

private:
	static const int sectionSize = 16 * 1024;

	struct Section
	{
		char data[sectionSize];
		int length;
		int modificationCount;

		Section() : length(0), modificationCount(0) {}
	};

	struct Modification
	{
		char before, after;
		int sectionIndex;

		Modification() {}

		Modification(char before, char after, int sectionIndex)
			: before(before), after(after), sectionIndex(sectionIndex) {}
	};

	QIODevice *m_device;
	QMap<int, Section> m_sections;
	int m_sectionIndex;
	int m_localPosition;
	qint64 m_absolutePosition;
	QMap<int, Section>::iterator m_section;
	QVector<Modification> m_modifications;
	qint64 m_size;
	int m_modificationCount;

	QMap<int, Section>::iterator loadSection(int sectionIndex);
	QMap<int, Section>::iterator getSection(int sectionIndex);
};

#endif // BUFFEREDEDITOR_H
