#include "hexview.h"

#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QtGlobal>
#include <QFontDatabase>
#include <QMenu>
#include <QAction>
#include <QClipboard>
#include <QGuiApplication>
#include <QApplication>
#include <QStyle>
#include <QScrollBar>
#include <QtMath>
#include <QMessageBox>

#include <QDebug>

static QColor backgroundColor("#ffffff");
static QColor alternateBackgroundColor("#aaaaaa");
static QColor textColor("#000000");
static QColor hoverTextColor("#ff0000");
static QColor selectedColor("#0000ff");
static QColor selectedTextColor("#000000");

#define cellX(x) (lineNumberWidth() + (x) * m_cellSize + ((x) + 1 + ((x) / 8)) * m_cellPadding)
#define textX(x) (cellX(m_bytesPerLine + 1) + x * (m_characterWidth + 5))

static const char hexTable[16] = {'0', '1', '2', '3', '4', '5', '6', '7',
								  '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

HexView::HexView(QWidget *parent)
	: QWidget(parent)
	, m_font(QFontDatabase::systemFont(QFontDatabase::SystemFont::FixedFont))
	, m_fontMetrics(m_font)
#if QT_VERSION >= 0x050B00
	, m_characterWidth(m_fontMetrics.horizontalAdvance(' '))
#else
	, m_characterWidth(m_fontMetrics.width(' '))
#endif
	, m_cellSize(m_fontMetrics.height())
	, m_cellPadding(m_characterWidth)
	, m_bytesPerLine(16)
	, m_hoveredIndex(-1)
	, m_selectionStart(-1)
	, m_selectionEnd(-1)
	, m_selection(Selection::None)
	, m_selecting(false)
	, m_editor(nullptr)
	, m_verticalScrollBar(new QScrollBar(this))
	, m_scrollTopRow(0)
	, m_mouseScrollBuffer(0.0)
	, m_editingCell(-1)
	, m_editingCellByte(0x00)
{
	m_verticalScrollBar->resize(QApplication::style()->pixelMetric(QStyle::PM_ScrollBarExtent), height());
	connect(m_verticalScrollBar, &QAbstractSlider::valueChanged, this, &HexView::setVerticalScrollPosition);

	QPalette pal = palette();
	backgroundColor = pal.base().color();
	alternateBackgroundColor = pal.alternateBase().color();
	textColor = pal.text().color();
	hoverTextColor = pal.link().color();
	selectedColor = pal.highlight().color();
	selectedTextColor = pal.highlightedText().color();

	pal.setColor(QPalette::Window, backgroundColor);
	setAutoFillBackground(true);
	setPalette(pal);
	setFixedWidth(textX(m_bytesPerLine) + m_cellPadding + m_verticalScrollBar->width());
	setMinimumHeight(80);
	setMouseTracking(true);
	setFocusPolicy(Qt::WheelFocus);
}

QString HexView::toPlainText()
{
	QString s;
	if (m_editor->isEmpty())
		return s;

	QString byte = "FF ";
	m_editor->seek(0);
	while (!m_editor->atEnd()) {
		for (int x = 0; !m_editor->atEnd() && x < m_bytesPerLine; ++x) {
			unsigned char b = static_cast<unsigned char>(m_editor->getByte());
			byte[0] = hexTable[(b >> 4) & 0xF];
			byte[1] = hexTable[(b >> 0) & 0xF];
			s.append(byte);
		}
		s[s.size() - 1] = '\n';
	}
	return s;
}

QPoint HexView::getByteCoordinates(int index) const
{
	QPoint p;
	p.setX(cellX(index % 16));
	p.setY((index / 16) * (m_cellSize + m_cellPadding) + m_cellSize - m_fontMetrics.ascent());
	return p;
}

std::optional<HexView::ByteSelection> HexView::selection() const
{
	if (m_selection == Selection::None)
		return std::optional<ByteSelection>();
	int s = m_selectionStart, e = m_selectionEnd;
	if (s > e)
		qSwap(s, e);
	return ByteSelection(s, e - s + 1);
}

qint64 HexView::rowCount() const
{
	return m_editor->size() / m_bytesPerLine + (m_editor->size() % m_bytesPerLine > 0);
}

bool HexView::canUndo() const
{
	return m_editor->canUndo();
}

void HexView::setBytesPerLine(int bytesPerLine)
{
	/*
	m_bytesPerLine = bytesPerLine;

	setFixedWidth(textX(m_bytesPerLine) + m_cellPadding);
	int rows = m_data.size() / m_bytesPerLine + (m_data.size() % m_bytesPerLine > 0);
	int widgetHeight = rows * m_cellSize + (rows + 1) * m_cellPadding;
	if (widgetHeight != height())
		resize(width(), widgetHeight);
	repaint();
	*/
}

void HexView::highlight(ByteSelection selection)
{
	m_selection = Selection::Cells;
	m_selectionStart = selection.begin;
	m_selectionEnd = selection.begin + selection.count - 1;

	repaint();
}

void HexView::selectNone()
{
	m_selectionStart = 0;
	m_selectionEnd = 0;
	m_selection = Selection::None;
	m_selecting = false;

	repaint();
}

void HexView::setFont(QFont font)
{
	m_font = font;
	m_fontMetrics = QFontMetrics(m_font);
	m_characterWidth = m_fontMetrics.averageCharWidth();
	m_cellSize = m_fontMetrics.height();
	m_cellPadding = m_characterWidth;
	setFixedWidth(textX(m_bytesPerLine) + m_cellPadding);

	repaint();
}

void HexView::setVerticalScrollPosition(int topRow)
{
	topRow = qBound(0, topRow, int(rowCount())); // TODO: qint64
	m_verticalScrollBar->setValue(topRow);
	m_scrollTopRow = topRow;

	repaint();
}

bool HexView::openFile(const QString &path)
{
	m_file.setFileName(path);
	if (!m_file.open(QIODevice::ReadWrite)) {
		QMessageBox::critical(this, "",
							  QString("Failed to open file %1: %2").arg(path).arg(m_file.errorString()));
		m_editor.reset();
		return false;
	}

	m_editor = std::make_shared<BufferedEditor>(&m_file);

	m_verticalScrollBar->setRange(0, int(rowCount() - 1)); // TODO: qint64
	setVerticalScrollPosition(0);
	setFixedWidth(textX(m_bytesPerLine) + m_cellPadding + m_verticalScrollBar->width());
	selectNone();
	repaint();

	return true;
}


bool HexView::saveChanges()
{
	bool ok = m_editor->writeChanges();
	if (!ok)
		QMessageBox::critical(this, "",
							  QString("Failed to save file %1: %2").arg(m_file.fileName()).arg(m_editor->errorString()));
	return ok;
}

bool HexView::quit()
{
	if (!m_editor->isModified())
		return true;

	auto button = QMessageBox::question(this, "",
										QString("Save changes to %1?").arg(m_file.fileName()),
										QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
	if (button == QMessageBox::Yes) {
		if (!saveChanges())
			return false;
	} else if (button == QMessageBox::Cancel) {
		return false;
	}

	return true;
}

void HexView::undo()
{
	m_editor->undo();
	emit canUndoChanged(canUndo());
	repaint();
}

void HexView::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);

	painter.setFont(m_font);

	const int cellHeight = m_cellSize + m_cellPadding;

	const int startY = qMax(0, (event->rect().y() + cellHeight * int(m_scrollTopRow)) / cellHeight); // TODO: qint64
	const int endY = qMin(int(m_editor->size()), (event->rect().bottom() + cellHeight * int(m_scrollTopRow)) / cellHeight + 1); // TODO: qint64

	int selectionStart = -1;
	int selectionEnd = -1;
	if (m_selection == Selection::Cells || m_selection == Selection::Text) {
		selectionStart = qMin(m_selectionStart, m_selectionEnd);
		selectionEnd = qMax(m_selectionStart, m_selectionEnd) + 1;
	} else if (m_selection == Selection::CellRows || m_selection == Selection::TextRows) {
		if (m_selectionStart < m_selectionEnd) {
			selectionStart = m_selectionStart;
			selectionEnd = m_selectionEnd + 1;
		} else {
			selectionStart = m_selectionEnd - m_bytesPerLine + 1;
			selectionEnd = m_selectionStart + m_bytesPerLine;
		}
	}

	QString cellText = "FF";
	QString ch = "a";
	int i = startY * m_bytesPerLine;
	m_editor->seek(i);
	for (int y = startY, yCoord = m_cellSize; i < m_editor->size() && y < endY; ++y, yCoord += cellHeight) {

		bool rowIsHovered = m_hoveredIndex == -1 ? false : m_hoveredIndex / 16 == y;

		painter.setPen(rowIsHovered ? textColor : alternateBackgroundColor);
		painter.setBrush(y % 2 == 0 ? backgroundColor : alternateBackgroundColor);
		painter.drawRect(0, yCoord - m_fontMetrics.ascent() - m_cellPadding / 2, width(), cellHeight - 1);

		painter.setPen(rowIsHovered ? hoverTextColor : textColor);
		painter.drawText(QPointF(m_cellSize / 2, yCoord),
						 QString::number(m_editor->position(), 16).rightJustified(lineNumberDigitsCount(), '0'));

		for (int x = 0; i < m_editor->size() && x < m_bytesPerLine; ++x, ++i) {
			unsigned char byte = static_cast<unsigned char>(m_editor->getByte());
			cellText[0] = hexTable[(byte >> 4) & 0xF];
			cellText[1] = hexTable[(byte >> 0) & 0xF];
			ch[0] = (byte >= 32 && byte <= 126) ? char(byte) : '.';

			QPoint cellCoord;
			cellCoord.setX(cellX(x));
			cellCoord.setY(yCoord);

			QPoint textCoord;
			textCoord.setX(textX(x));
			textCoord.setY(yCoord);

			bool inSelection = (i >= selectionStart && i < selectionEnd);

			if (i == m_hoveredIndex || inSelection) {
				if (inSelection)
					painter.setBrush(selectedColor);
				else
					painter.setBrush(backgroundColor);
				if (i == m_hoveredIndex)
					painter.setPen(hoverTextColor);
				else
					painter.setPen(selectedColor);

				painter.drawRect(textCoord.x() - 2,
								 textCoord.y() - m_fontMetrics.ascent() - 2,
								 m_characterWidth + 4,
								 m_fontMetrics.height() + 4);

				if (m_editingCell == i) {
					painter.setPen(textColor);
					painter.setBrush(backgroundColor);
				}
				painter.drawRect(cellCoord.x() - m_cellPadding / 2,
								 cellCoord.y() - m_fontMetrics.ascent() - m_cellPadding / 2,
								 m_characterWidth * 2 + m_cellPadding,
								 m_cellSize + m_cellPadding - 1);
			}

			if (m_hoveredIndex == i)
				painter.setPen(hoverTextColor);
			else
				painter.setPen(textColor);

			if (m_editingCell != i)
				painter.drawText(cellCoord, cellText);
			else
				painter.drawText(cellCoord.x() + m_characterWidth / 2, cellCoord.y(),
								 QString::number(m_editingCellByte, 16).toUpper());
			painter.drawText(textCoord, ch);
		}
	}

	{
		painter.setPen(textColor);
		int x = lineNumberWidth();
		qint64 y = qMin((rowCount() - m_scrollTopRow) * cellHeight, qint64(height()));
		painter.drawLine(x, 0, x, int(y));
	}
}

void HexView::resizeEvent(QResizeEvent *)
{
	// TODO

	m_verticalScrollBar->move(width() - m_verticalScrollBar->width(), 0);
	m_verticalScrollBar->resize(m_verticalScrollBar->width(), height());
}

void HexView::mouseMoveEvent(QMouseEvent *event)
{
	int hoverCellIndex = getHoverCell(event->pos());
	int hoverTextIndex = getHoverText(event->pos());

	int newIndex = qMax(hoverCellIndex, hoverTextIndex);
	if (newIndex != m_hoveredIndex) {
		m_hoveredIndex = newIndex;

		if (m_hoveredIndex != -1) {
			if (m_selecting) {
				if (newIndex == hoverCellIndex && (m_selection == Selection::Cells || m_selection == Selection::CellRows)) {
					if (m_selection == Selection::Cells)
						m_selectionEnd = m_hoveredIndex;
					else if (m_selection == Selection::CellRows)
						m_selectionEnd = m_bytesPerLine * (m_hoveredIndex / m_bytesPerLine) + m_bytesPerLine - 1;
			} else if (newIndex == hoverTextIndex && (m_selection == Selection::Text || m_selection == Selection::TextRows)) {
					if (m_selection == Selection::Text)
						m_selectionEnd = m_hoveredIndex;
					else if (m_selection == Selection::TextRows)
						m_selectionEnd = m_bytesPerLine * (m_hoveredIndex / m_bytesPerLine) + m_bytesPerLine - 1;
				}
			}
		}

		repaint();
	}
}

void HexView::mousePressEvent(QMouseEvent *event)
{
	m_editingCell = -1;

	if (event->button() == Qt::RightButton) {
		int selectionStart = -1;
		int selectionEnd = -1;
		if (m_selection == Selection::Cells || m_selection == Selection::Text) {
			selectionStart = qMin(m_selectionStart, m_selectionEnd);
			selectionEnd = qMax(m_selectionStart, m_selectionEnd) + 1;
		} else if (m_selection == Selection::CellRows || m_selection == Selection::TextRows) {
			if (m_selectionStart < m_selectionEnd) {
				selectionStart = m_selectionStart;
				selectionEnd = m_selectionEnd + 1;
			} else {
				selectionStart = m_selectionEnd - m_bytesPerLine + 1;
				selectionEnd = m_selectionStart + m_bytesPerLine;
			}
		}
		if (selectionEnd > m_editor->size())
			selectionEnd = m_editor->size();

		QMenu menu(this);
		QAction copyTextAction("Copy text");
		QAction copyHexAction("Copy hex");
		QAction selectAllAction("Select All");
		QAction selectNoneAction("Select None");

		menu.addAction(&copyTextAction);
		menu.addAction(&copyHexAction);
		menu.addSeparator();
		menu.addAction(&selectAllAction);
		menu.addAction(&selectNoneAction);

		menu.popup(event->globalPos());

		bool hasSelection = (selectionStart < selectionEnd);
		copyTextAction.setEnabled(hasSelection);
		copyHexAction.setEnabled(hasSelection);
		selectAllAction.setEnabled(!m_editor->isEmpty());
		selectNoneAction.setEnabled(hasSelection);

		QAction *a = menu.exec();

		if (a == &copyTextAction) {
			QString s;
			m_editor->seek(selectionStart);
			while (m_editor->position() < selectionEnd) {
				char b = m_editor->getByte();
				s.append((b >= 32 && b <= 126) ? b : '.');
			}
			QClipboard *clipboard = QGuiApplication::clipboard();
			clipboard->setText(s);
		} else if (a == &copyHexAction) {
			QString cell = "00 ";
			QString s;
			m_editor->seek(selectionStart);
			while (m_editor->position() < selectionEnd) {
				unsigned char byte = static_cast<unsigned char>(m_editor->getByte());
				cell[0] = hexTable[(byte >> 4) & 0xF];
				cell[1] = hexTable[(byte >> 0) & 0xF];
				s.append(cell);
			}
			s.remove(s.size() - 1, 1);
			QClipboard *clipboard = QGuiApplication::clipboard();
			clipboard->setText(s);
		} else if (a == &selectAllAction) {
			m_selectionStart = 0;
			m_selectionEnd = m_editor->size() - 1;
			m_selection = Selection::Cells;
			m_selecting = false;
			repaint();
		} else if (a == &selectNoneAction) {
			selectNone();
		}
		return;
	}
	int hoverCellIndex = getHoverCell(event->pos());
	int hoverTextIndex = getHoverText(event->pos());
	int newIndex = qMax(hoverCellIndex, hoverTextIndex);
	m_selectionStart = newIndex;
	m_selectionEnd = m_selectionStart;
	if (newIndex == -1)
		m_selection = Selection::None;
	else if (newIndex == hoverCellIndex)
		m_selection = Selection::Cells;
	else if (newIndex == hoverTextIndex)
		m_selection = Selection::Text;
	m_selecting = (newIndex != -1);
	repaint();
}

void HexView::mouseReleaseEvent(QMouseEvent *)
{
	m_selecting = false;
}

void HexView::mouseDoubleClickEvent(QMouseEvent *event)
{
	m_editingCell = -1;

	int hoverCellIndex = getHoverCell(event->pos());
	int hoverTextIndex = getHoverText(event->pos());
	int newIndex = qMax(hoverCellIndex, hoverTextIndex);
	if (newIndex != -1) {
		m_selectionStart = m_bytesPerLine * (newIndex / m_bytesPerLine);
		m_selectionEnd = m_selectionStart + m_bytesPerLine - 1;
		if (newIndex == hoverCellIndex)
			m_selection = Selection::CellRows;
		else
			m_selection = Selection::TextRows;
		m_selecting = true;
		repaint();
	}
}

void HexView::leaveEvent(QEvent *)
{
	m_hoveredIndex = -1;
	repaint();
}

void HexView::wheelEvent(QWheelEvent *event)
{
	QPoint p = event->angleDelta();
	if (p.y() != 0) {
		m_mouseScrollBuffer += p.y() / 120.0;
		int v = qFloor(m_mouseScrollBuffer);
		if (v != 0) {
			m_mouseScrollBuffer -= v;
			qint64 newTopRow = m_scrollTopRow;
			newTopRow -= v;
			setVerticalScrollPosition(newTopRow);
		}
	}
}

void HexView::keyPressEvent(QKeyEvent *event)
{
	if (m_selecting || m_selectionStart != m_selectionEnd) {
		QWidget::keyPressEvent(event);
		return;
	}

	int key = event->key();
	static const QSet<int> hexDigits = {
		'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
		'A', 'B', 'C', 'D', 'E', 'F',
	};
	bool keyIsHexDigit = hexDigits.contains(key);
	auto putByte = [this]() {
		m_editor->seek(m_editingCell);
		m_editor->putByte(m_editingCellByte);
		emit canUndoChanged(canUndo());
		m_editingCell = -1;
	};

	static const QSet<int> movingKeys = {
		Qt::Key_Enter, Qt::Key_Tab,
		Qt::Key_Up, Qt::Key_Down, Qt::Key_Left, Qt::Key_Right,
	};

	if (m_selection == Selection::Cells) {
		if (keyIsHexDigit) {
			if (m_editingCell == -1) {
				m_editingCell = m_selectionStart;
				m_editingCellByte = char(QString(char(key)).toInt(nullptr, 16));
				repaint();
				return;
			} else {
				m_editingCellByte <<= 4;
				m_editingCellByte |= char(QString(char(key)).toInt(nullptr, 16));
				putByte();
				++m_selectionStart;
				++m_selectionEnd;
				repaint();
				return;
			}
		} else if (key == Qt::Key_Escape) {
			if (m_editingCell != -1) {
				m_editingCell = -1;
				repaint();
				return;
			}
		}
	} else if (m_selection == Selection::Text) {
		QString text = event->text();
		if (text.size() == 1) {
			char byte = text[0].toLatin1();
			if (byte >= 32 && byte < 127) {
				m_editor->seek(m_selectionStart);
				m_editor->putByte(byte);
				emit canUndoChanged(canUndo());
				++m_selectionStart;
				++m_selectionEnd;
				repaint();
				return;
			}
		}
	}

	if (movingKeys.contains(key)) {
		if (m_editingCell != -1)
			putByte();
		int move = 0;
		switch (key) {
		case Qt::Key_Enter:
		case Qt::Key_Tab:
		case Qt::Key_Right:
			move = 1; break;
		case Qt::Key_Left:
			move = -1; break;
		case Qt::Key_Up:
			move = -m_bytesPerLine; break;
		case Qt::Key_Down:
			move = m_bytesPerLine; break;
		}
		m_selectionStart = qBound(0, m_selectionStart + move, int(m_editor->size() - 1)); // TODO: qint64
		m_selectionEnd = m_selectionStart;
		repaint();
		return;
	}

	QWidget::keyPressEvent(event);
}

int HexView::getHoverCell(const QPoint &mousePos) const
{
	int x = mousePos.x();
	int y = mousePos.y();

	x -= lineNumberWidth();

	{
		int vx = x;
		while (vx > 8 * m_cellPadding + 8 * m_cellSize) {
			x -= m_cellPadding;
			vx -= m_cellPadding;
			vx -= 8 * m_cellPadding + 8 * m_cellSize;
		}
	}

	x -= m_cellPadding / 2;
	y -= m_cellPadding / 2;

	y += (m_cellPadding + m_cellSize) * m_scrollTopRow;

	int xi = -1;
	int yi = -1;

	if (x >= 0 && x < m_bytesPerLine * (m_cellPadding + m_cellSize))
		xi = x / (m_cellPadding + m_cellSize);

	if (y >= 0 && y < m_editor->size() * (m_cellPadding + m_cellSize))
		yi = y / (m_cellPadding + m_cellSize);

	if (xi != -1 && yi != -1)
		return xi + m_bytesPerLine * yi;

	return -1;
}

int HexView::getHoverText(const QPoint &mousePos) const
{
	int x = mousePos.x();
	int y = mousePos.y();

	y += (m_cellPadding + m_cellSize) * m_scrollTopRow;

	x -= cellX(m_bytesPerLine + 1);

	int xi = -1, yi = -2;
	if (x >= 0 && x < (m_characterWidth + 5) * m_bytesPerLine)
		xi = x / (m_characterWidth + 5);
	if (y >= 0 && y < m_editor->size() * (m_cellPadding + m_cellSize))
		yi = y / (m_cellPadding + m_cellSize);

	if (xi != -1 && yi != -1)
		return xi + m_bytesPerLine * yi;

	return -1;
}

int HexView::lineNumberDigitsCount() const
{
	if (m_editor == nullptr)
		return 0;

	int digits = 1;
	qint64 size = m_editor->size();
	while (size /= 16)
		++digits;
	return digits;
}

int HexView::lineNumberWidth() const
{
	return m_cellSize + lineNumberDigitsCount() * m_characterWidth;
}
