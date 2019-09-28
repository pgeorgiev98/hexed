#ifndef HEXVIEW_H
#define HEXVIEW_H

#include "bufferededitor.h"

#include <QWidget>
#include <QString>
#include <QByteArray>
#include <QFont>
#include <QFontMetrics>
#include <QMap>
#include <QFile>

#include <optional>
#include <memory>

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

	QString toPlainText();
	QPoint getByteCoordinates(int index) const;
	std::optional<ByteSelection> selection() const;
	qint64 rowCount() const;

public slots:
	void setBytesPerLine(int bytesPerLine);
	void highlight(ByteSelection selection);
	void selectNone();
	void setFont(QFont font);
	void setVerticalScrollPosition(int topRow);
	bool openFile(const QString &path);
	bool saveChanges();
	bool quit();

protected:
	void paintEvent(QPaintEvent *) override;
	void resizeEvent(QResizeEvent *event) override;
	void mouseMoveEvent(QMouseEvent *) override;
	void mousePressEvent(QMouseEvent *) override;
	void mouseReleaseEvent(QMouseEvent *) override;
	void mouseDoubleClickEvent(QMouseEvent *) override;
	void leaveEvent(QEvent *) override;
	void wheelEvent(QWheelEvent *) override;
	void keyPressEvent(QKeyEvent *) override;

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
	QFile m_file;
	std::shared_ptr<BufferedEditor> m_editor;
	QScrollBar *m_verticalScrollBar;
	qint64 m_scrollTopRow;
	double m_mouseScrollBuffer;
	int m_editingCell;
	char m_editingCellByte;

	int getHoverCell(const QPoint &mousePos) const;
	int getHoverText(const QPoint &mousePos) const;
	int lineNumberDigitsCount() const;
	int lineNumberWidth() const;
};

#endif // HEXVIEW_H
