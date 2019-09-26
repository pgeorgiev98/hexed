#ifndef HEXVIEW_H
#define HEXVIEW_H

#include <QWidget>
#include <QString>
#include <QByteArray>
#include <QFont>
#include <QFontMetrics>

#include <optional>

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

public slots:
	void clear();
	void setData(const QByteArray &data);
	void insertData(const QByteArray &data);
	void setBytesPerLine(int bytesPerLine);
	void highlight(ByteSelection selection);
	void selectNone();
	void setFont(QFont font);

signals:
	void highlightInTextView(ByteSelection selection);

protected:
	void paintEvent(QPaintEvent *) override;
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
	QByteArray m_data;
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

	int getHoverCell(const QPoint &mousePos) const;
	int getHoverText(const QPoint &mousePos) const;
};

#endif // HEXVIEW_H
