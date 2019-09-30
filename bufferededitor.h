#ifndef BUFFEREDEDITOR_H
#define BUFFEREDEDITOR_H

#include <QObject>
#include <QMap>
#include <QVector>

#include <variant>

class QFileDevice;

class BufferedEditor : public QObject
{
	Q_OBJECT
public:
	BufferedEditor(QFileDevice *device, QObject *parent = nullptr);
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
	bool canUndo() const;
	void undo();

signals:
	void canUndoChanged(bool canUndo);

private:
	static const int sectionSize = 16 * 1024;

	struct Section
	{
		char data[sectionSize];
		int length;
		int modificationCount;

		Section() : length(0), modificationCount(0) {}
	};

	struct Replacement
	{
		char before, after;
		quint16 localPosition;
		int sectionIndex;

		Replacement() {}
		Replacement(char before, char after, quint16 localPosition, int sectionIndex)
			: before(before), after(after), localPosition(localPosition), sectionIndex(sectionIndex) {}
	};

	struct Insertion
	{
		char byte;
		quint16 localPosition;
		int sectionIndex;

		Insertion() {}
		Insertion(char byte, quint16 localPosition, int sectionIndex)
			: byte(byte), localPosition(localPosition), sectionIndex(sectionIndex) {}
	};

	typedef std::variant<Replacement, Insertion> Modification;

	QFileDevice *m_device;
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
