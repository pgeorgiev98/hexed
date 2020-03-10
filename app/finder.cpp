#include "finder.h"
#include "bufferededitor.h"

Finder::Finder(BufferedEditor *editor, QObject *parent)
	: QObject(parent)
	, m_editor(editor)
	, m_position(-1)
	, m_searchResultPosition(-1)
{
}

void Finder::search(qint64 position, const QByteArray &searchData)
{
	m_position = position;
	m_searchData = searchData;

	// Build an automata that matches searchData
	// It is presented as a table with 256 columns
	// searchData.size() rows.

	// It will be built such that its [n*256+i]th element
	// (nth row, ith column) is the row you have to go to
	// if you're on the nth row and read a byte with a value of i
	m_automata.resize(m_searchData.size() * 256);

	for (int n = 0; n < m_searchData.size(); ++n) {
		QByteArray s = m_searchData.left(n);
		for (int i = 0; i < 256; ++i) {
			QByteArray ss = s + char(i);
			int t = n + 1;
			while (!m_searchData.startsWith(ss.right(t)))
				--t;
			m_automata[n * 256 + i] = t;
		}
	}
	m_automataState = 0;
}

void Finder::findNext()
{
	if (m_automataState == m_searchData.size())
		m_automataState = 0;
	m_editor->seek(m_position);
	while (m_position < m_editor->size() && m_automataState < m_searchData.size()) {
		auto byte = m_editor->getByte();
		m_automataState = m_automata[m_automataState * 256 + (unsigned char)*byte.current];
		++m_position;
	}
	m_searchResultPosition = m_automataState == m_searchData.size() ? m_position - m_searchData.size() : -1;
	emit searchFinished(m_searchResultPosition);
}

void Finder::findPrevious()
{
	// TODO
	Q_ASSERT_X(false, "Finder::findPrevious()", "Not implemented");
}

const QByteArray &Finder::searchData() const
{
	return m_searchData;
}

qint64 Finder::searchResultPosition() const
{
	return m_searchResultPosition;
}
