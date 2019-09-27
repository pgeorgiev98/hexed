#ifndef HEXVIEW_H
#define HEXVIEW_H

#include "bufferededitor.h"

#include <QWidget>
#include <QString>
#include <QByteArray>
#include <QFont>
#include <QFontMetrics>
#include <QMap>

#include <optional>

class QScrollBar;

class HexView : public QWidget
{
	Q_OBJECT
public:
	explicit HexView(QWidget *parent = nullptr);

	struct ByteSelection
	{
		int begin, count;
		ByteSelection(int begin, int count)
			: begin(begin), count(count) {}
	};

	QString toPlainText() const;
	QPoint getByteCoordinates(int index) const;
	std::optional<ByteSelection> selection() const;
	qint64 rowCount() const;

public slots:
	void setBytesPerLine(int bytesPerLine);
	void highlight(ByteSelection selection);
	void selectNone();
	void setFont(QFont font);
	void setEditor(BufferedEditor *editor);
	void setVerticalScrollPosition(int topRow);

signals:
	void highlightInTextView(ByteSelection selection);

protected:
	void paintEvent(QPaintEvent *) override;
	void resizeEvent(QResizeEvent *event) override;
	void mouseMoveEvent(QMouseEvent *) override;
	void mousePressEvent(QMouseEvent *) override;
	void mouseReleaseEvent(QMouseEvent *) override;
	void mouseDoubleClickEvent(QMouseEvent *) override;
	void leaveEvent(QEvent *) override;

private:
	QFont m_font;
	QFontMetrics m_fontMetrics;
	int m_characterWidth;
	int m_cellSize, m_cellPadding;
	int m_bytesPerLine;
	int m_hoveredIndex;
	int m_selectionStart;
	int m_selectionEnd;
	enum class Selection {
		None = 0,
		Cells,
		CellRows,
		Text,
		TextRows,
	} m_selection;
	bool m_selecting;
	BufferedEditor *m_editor;
	QScrollBar *m_verticalScrollBar;
	qint64 m_scrollTopRow;

	int getHoverCell(const QPoint &mousePos) const;
	int getHoverText(const QPoint &mousePos) const;
	int lineNumberDigitsCount() const;
	int lineNumberWidth() const;
};

#endif // HEXVIEW_H
