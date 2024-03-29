#ifndef BUFFEREDEDITOR_H
#define BUFFEREDEDITOR_H

#include <QObject>
#include <QVector>

#include <variant>
#include <optional>

class QFileDevice;

class BufferedEditor : public QObject
{
	Q_OBJECT
public:
	struct Byte
	{
		std::optional<char> saved;
		std::optional<char> current;

		bool isModified() const
		{
			return !saved || *saved != current;
		}

		Byte() {}

		Byte(std::optional<char> saved, std::optional<char> current)
			: saved(saved), current(current) {}
	};

	BufferedEditor(QFileDevice *device, QObject *parent = nullptr);
	QString errorString() const;
	bool seek(qint64 position);
	qint64 position() const;
	qint64 size() const;
	bool isEmpty() const;
	bool atEnd() const;
	void moveForward();
	Byte getByte();
	void replaceByte(char byte);
	void insertByte(char byte);
	void deleteByte();
	bool writeChanges();
	bool isModified() const;
	bool canUndo() const;
	bool canRedo() const;
	void undo();
	void redo();

signals:
	void canUndoChanged(bool canUndo);
	void canRedoChanged(bool canRedo);
	void sizeChanged(qint64 size);

private:
	static const int sectionSize = 16 * 1024;

	struct Section
	{
		qint64 savedPosition;
		qint64 currentPosition;
		QVector<Byte> data;
		int modificationCount;

		bool isModified() const
		{
			return modificationCount != 0;
		}

		int savedLength() const
		{
			int l = 0;
			for (Byte b : data)
				l += b.saved.has_value();
			return l;
		}

		int currentLength() const
		{
			int l = 0;
			for (Byte b : data)
				l += b.current.has_value();
			return l;
		}

		int bytePosition(qint64 position) const
		{
			qint64 p = currentPosition;
			int i = 0;
			if (p > position)
				return -1;
			for (Byte b : data) {
				if (p == position && b.current.has_value())
					break;
				p += b.current.has_value();
				++i;
			}
			if (i == data.size())
				return -1;
			return i;
		}

		Section() : savedPosition(-1), currentPosition(-1), modificationCount(0) {}
		Section(qint64 savedPosition, qint64 currentPosition)
			: savedPosition(savedPosition), currentPosition(currentPosition), modificationCount(0) {}
	};

	struct Modification
	{
		enum class Type
		{
			Replace, Insert, Delete
		};

		Type type;
		char byte;
		int sectionIndex, byteIndex;

		Modification(Type type, char byte, int sectionIndex, int byteIndex)
			: type(type)
			, byte(byte)
			, sectionIndex(sectionIndex)
			, byteIndex(byteIndex)
		{
		}
	};

	QFileDevice *m_device;
	QVector<Section> m_sections;
	int m_sectionIndex;
	int m_sectionLocalPosition;
	qint64 m_position;
	qint64 m_size;
	QVector<Modification> m_modifications;
	int m_currentModificationIndex;
	int m_modificationCount;

	int getSectionIndex(qint64 position);
	void doModification(Modification &modification);
	void undoModification(Modification &modification);
	void userDoModification(Modification m);
	void updateSectionsPosition(int firstSectionIndex);
};

#endif // BUFFEREDEDITOR_H
