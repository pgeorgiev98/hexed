#ifndef FINDER_H
#define FINDER_H

#include <QObject>

class BufferedEditor;

class Finder : public QObject
{
	Q_OBJECT
public:
	explicit Finder(BufferedEditor *editor, QObject *parent = nullptr);

signals:
	void searchFinished(qint64 position);

public slots:
	void search(qint64 position, const QByteArray &searchData);
	void findNext();
	void findPrevious();
	const QByteArray &searchData() const;
	qint64 searchResultPosition() const;

private:
	BufferedEditor *m_editor;
	qint64 m_position;
	QByteArray m_searchData;
	QVector<int> m_automata;
	int m_automataState;
	qint64 m_searchResultPosition;
};

#endif // FINDER_H
