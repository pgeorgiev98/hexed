#include "hexviewinternal.h"
#include "gotodialog.h"
#include "findwidget.h"

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
static QColor modifiedTextColor("#ff0000");
static QColor selectedColor("#0000ff");
static QColor selectedTextColor("#000000");

#define cellX(x) (lineNumberWidth() + (x) * m_cellSize + ((x) + 1 + ((x) / 8)) * m_cellPadding)
#define textX(x) (cellX(m_bytesPerLine + 1) + x * (m_characterWidth + 5))

static const char hexTable[16] = {'0', '1', '2', '3', '4', '5', '6', '7',
								  '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

HexViewInternal::HexViewInternal(QWidget *parent)
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
	, m_topRow(0)
	, m_mouseScrollBuffer(0.0)
	, m_editingCell(false)
	, m_editingCellByte(0x00)
	, m_gotoDialog(new GotoDialog(this))
	, m_findWidget(new FindWidget(this))
{
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
	setFixedWidth(textX(m_bytesPerLine) + m_cellPadding);
	setMinimumHeight(80);
	setMouseTracking(true);
	setFocusPolicy(Qt::WheelFocus);

	m_findWidget->hide();

	connect(m_findWidget, &FindWidget::closed, [this]() { update(); });
}

QString HexViewInternal::toPlainText()
{
	QString s;
	if (m_editor->isEmpty())
		return s;

	QString byte = "FF ";
	m_editor->seek(0);
	while (!m_editor->atEnd()) {
		for (int x = 0; !m_editor->atEnd() && x < m_bytesPerLine; ++x) {
			unsigned char b = static_cast<unsigned char>(*m_editor->getByte().current);
			byte[0] = hexTable[(b >> 4) & 0xF];
			byte[1] = hexTable[(b >> 0) & 0xF];
			s.append(byte);
		}
		s[s.size() - 1] = '\n';
	}
	return s;
}

QPoint HexViewInternal::getByteCoordinates(qint64 index) const
{
	QPoint p;
	p.setX(cellX(index % 16));
	p.setY((index / 16) * (m_cellSize + m_cellPadding) + m_cellSize - m_fontMetrics.ascent());
	return p;
}

std::optional<HexViewInternal::ByteSelection> HexViewInternal::selection() const
{
	if (m_selection == Selection::None)
		return std::optional<ByteSelection>();
	qint64 s = m_selectionStart, e = m_selectionEnd;
	if (s > e)
		qSwap(s, e);
	return ByteSelection(s, e - s + 1);
}

qint64 HexViewInternal::rowCount() const
{
	return (m_editor->size() + 1) / m_bytesPerLine + ((m_editor->size() + 1) % m_bytesPerLine > 0);
}

qint64 HexViewInternal::scrollMaximum() const
{
	int displayedRows = height() / (m_cellSize + m_cellPadding);
	return rowCount() - displayedRows + 5;
}

bool HexViewInternal::canUndo() const
{
	return m_editor->canUndo();
}

bool HexViewInternal::canRedo() const
{
	return m_editor->canRedo();
}

bool HexViewInternal::cursorIsInFindWidget(QPoint cursorPos) const
{
	if (m_findWidget->isVisible()) {
		QPoint p = m_findWidget->mapFromParent(cursorPos);
		if (m_findWidget->rect().contains(p))
			return true;
	}
	return false;
}

void HexViewInternal::setBytesPerLine(int bytesPerLine)
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

void HexViewInternal::highlight(ByteSelection selection)
{
	m_selection = Selection::Cells;
	m_selectionStart = selection.begin;
	m_selectionEnd = selection.begin + selection.count - 1;

	repaint();
}

void HexViewInternal::selectNone()
{
	m_selectionStart = 0;
	m_selectionEnd = 0;
	m_selection = Selection::None;
	m_selecting = false;

	repaint();
}

void HexViewInternal::setFont(QFont font)
{
	m_font = font;
	m_fontMetrics = QFontMetrics(m_font);
	m_characterWidth = m_fontMetrics.averageCharWidth();
	m_cellSize = m_fontMetrics.height();
	m_cellPadding = m_characterWidth;
	setFixedWidth(textX(m_bytesPerLine) + m_cellPadding);

	repaint();
}

void HexViewInternal::setTopRow(qint64 topRow)
{
	topRow = qBound(qint64(0), topRow, rowCount());
	m_topRow = topRow;
	emit topRowChanged(topRow);

	repaint();
}

bool HexViewInternal::openFile(const QString &path)
{
	m_file.setFileName(path);
	if (!m_file.open(QIODevice::ReadWrite)) {
		QMessageBox::critical(this, "",
							  QString("Failed to open file %1: %2").arg(path).arg(m_file.errorString()));
		m_editor = nullptr;
		return false;
	}

	m_editor = new BufferedEditor(&m_file, this);

	setTopRow(0);
	setFixedWidth(textX(m_bytesPerLine) + m_cellPadding);
	selectNone();
	repaint();

	connect(m_editor, &BufferedEditor::canUndoChanged, this, &HexViewInternal::canUndoChanged);
	connect(m_editor, &BufferedEditor::canRedoChanged, this, &HexViewInternal::canRedoChanged);

	emit rowCountChanged();

	return true;
}


bool HexViewInternal::saveChanges()
{
	bool ok = m_editor->writeChanges();
	if (!ok)
		QMessageBox::critical(this, "",
							  QString("Failed to save file %1: %2").arg(m_file.fileName()).arg(m_editor->errorString()));
	repaint();
	return ok;
}

bool HexViewInternal::quit()
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

void HexViewInternal::undo()
{
	m_editor->undo();
	repaint();
}

void HexViewInternal::redo()
{
	m_editor->redo();
	repaint();
}

void HexViewInternal::openGotoDialog()
{
	m_gotoDialog->setMaximum(m_editor->size());
	if (!m_gotoDialog->exec())
		return;

	qint64 position = m_gotoDialog->position();
	m_selecting = false;
	if (m_selection == Selection::Text || m_selection == Selection::TextRows)
		m_selection = Selection::Text;
	else
		m_selection = Selection::Cells;
	m_selectionStart = m_selectionEnd = position;
	setTopRow(position / m_bytesPerLine);

	repaint();
}

void HexViewInternal::openFindDialog()
{
	m_findWidget->show();
	updateFindDialogPosition();
	m_findWidget->setFocus();
}

void HexViewInternal::updateFindDialogPosition()
{
	m_findWidget->move(0, height() - m_findWidget->height());
}

void HexViewInternal::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);

	painter.setFont(m_font);

	const int cellHeight = m_cellSize + m_cellPadding;

	const qint64 startY = qMax(qint64(0), (event->rect().y() + cellHeight * m_topRow) / cellHeight);
	const qint64 endY = qMin(m_editor->size(), (event->rect().bottom() + cellHeight * m_topRow) / cellHeight + 1);

	qint64 selectionStart = -1;
	qint64 selectionEnd = -1;
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
	qint64 i = startY * m_bytesPerLine;
	if (i >= m_editor->size())
		return;
	m_editor->seek(i);
	for (qint64 y = startY, yCoord = m_cellSize; i <= m_editor->size() && y < endY; ++y, yCoord += cellHeight) {

		bool rowIsHovered = m_hoveredIndex == -1 ? false : m_hoveredIndex / 16 == y;

		painter.setPen(rowIsHovered ? textColor : alternateBackgroundColor);
		painter.setBrush(y % 2 == 0 ? backgroundColor : alternateBackgroundColor);
		painter.drawRect(0, yCoord - m_fontMetrics.ascent() - m_cellPadding / 2, width(), cellHeight - 1);

		painter.setPen(rowIsHovered ? hoverTextColor : textColor);
		painter.drawText(QPointF(m_cellSize / 2, yCoord),
						 QString::number(m_editor->position(), 16).rightJustified(lineNumberDigitsCount(), '0'));

		for (qint64 x = 0; i <= m_editor->size() && x < m_bytesPerLine; ++x, ++i) {
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

				if (m_editingCell && i >= m_selectionStart && i <= m_selectionEnd) {
					painter.setPen(textColor);
					painter.setBrush(backgroundColor);
				}
				painter.drawRect(cellCoord.x() - m_cellPadding / 2,
								 cellCoord.y() - m_fontMetrics.ascent() - m_cellPadding / 2,
								 m_characterWidth * 2 + m_cellPadding,
								 m_cellSize + m_cellPadding - 1);
			}

			bool editingLast = m_editingCell && m_selectionStart == m_editor->size();
			if (!m_editor->atEnd() || editingLast) {
				bool isModified = false;
				unsigned char byte;
				if (!m_editor->atEnd()) {
					BufferedEditor::Byte b = m_editor->getByte();
					isModified = b.saved != b.current;
					byte = static_cast<unsigned char>(*b.current);
					cellText[0] = hexTable[(byte >> 4) & 0xF];
					cellText[1] = hexTable[(byte >> 0) & 0xF];
					ch[0] = (byte >= 32 && byte <= 126) ? char(byte) : '.';
				}


				if (isModified)
					painter.setPen(modifiedTextColor);
				else if (m_hoveredIndex == i)
					painter.setPen(hoverTextColor);
				else
					painter.setPen(textColor);

				if (!(m_editingCell && i >= m_selectionStart && i <= m_selectionEnd))
					painter.drawText(cellCoord, cellText);
				else
					painter.drawText(cellCoord.x() + m_characterWidth / 2, cellCoord.y(),
									 QString::number(m_editingCellByte, 16).toUpper());
				painter.drawText(textCoord, ch);
			}
		}
	}

	{
		painter.setPen(textColor);
		int x = lineNumberWidth();
		qint64 y = qMin((rowCount() - m_topRow) * cellHeight, qint64(height()));
		painter.drawLine(x, 0, x, int(y));
	}
}

void HexViewInternal::resizeEvent(QResizeEvent *)
{
	emit scrollMaximumChanged();
	updateFindDialogPosition();
}

void HexViewInternal::mouseMoveEvent(QMouseEvent *event)
{
	qint64 hoverCellIndex = getHoverCell(event->pos());
	qint64 hoverTextIndex = getHoverText(event->pos());

	qint64 newIndex = qMax(hoverCellIndex, hoverTextIndex);
	if (newIndex != m_hoveredIndex) {
		m_hoveredIndex = newIndex;

		if (m_hoveredIndex != -1 && m_selecting) {
			if (m_selectionStart == m_editor->size()) {
				// We can't selecting when starting with the EOF "byte"
				m_selectionEnd = m_selectionStart;
			} else {
				if ((newIndex == hoverCellIndex && m_selection == Selection::Cells) ||
						(newIndex == hoverTextIndex && m_selection == Selection::Text))
					m_selectionEnd = m_hoveredIndex;
				else if ((newIndex == hoverCellIndex && m_selection == Selection::CellRows) ||
						 (newIndex == hoverTextIndex && m_selection == Selection::TextRows))
					m_selectionEnd = m_bytesPerLine * (m_hoveredIndex / m_bytesPerLine) + m_bytesPerLine - 1;

				// Don't allow to select past the last byte in the file
				if (m_selectionEnd >= m_editor->size())
					m_selectionEnd = m_editor->size() - 1;
			}
		}

		repaint();
	}
}

void HexViewInternal::mousePressEvent(QMouseEvent *event)
{
	m_editingCell = false;

	if (event->button() == Qt::RightButton) {
		qint64 selectionStart = -1;
		qint64 selectionEnd = -1;
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
				char b = *m_editor->getByte().current;
				s.append((b >= 32 && b <= 126) ? b : '.');
			}
			QClipboard *clipboard = QGuiApplication::clipboard();
			clipboard->setText(s);
		} else if (a == &copyHexAction) {
			QString cell = "00 ";
			QString s;
			m_editor->seek(selectionStart);
			while (m_editor->position() < selectionEnd) {
				unsigned char byte = static_cast<unsigned char>(*m_editor->getByte().current);
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
	qint64 hoverCellIndex = getHoverCell(event->pos());
	qint64 hoverTextIndex = getHoverText(event->pos());
	qint64 newIndex = qMax(hoverCellIndex, hoverTextIndex);
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

void HexViewInternal::mouseReleaseEvent(QMouseEvent *)
{
	m_selecting = false;
}

void HexViewInternal::mouseDoubleClickEvent(QMouseEvent *event)
{
	m_editingCell = false;

	qint64 hoverCellIndex = getHoverCell(event->pos());
	qint64 hoverTextIndex = getHoverText(event->pos());
	qint64 newIndex = qMax(hoverCellIndex, hoverTextIndex);
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

void HexViewInternal::leaveEvent(QEvent *)
{
	m_hoveredIndex = -1;
	repaint();
}

void HexViewInternal::wheelEvent(QWheelEvent *event)
{
	QPoint p = event->angleDelta();
	if (p.y() != 0) {
		m_mouseScrollBuffer += p.y() / 120.0;
		int v = qFloor(m_mouseScrollBuffer);
		if (v != 0) {
			m_mouseScrollBuffer -= v;
			qint64 newTopRow = m_topRow;
			newTopRow -= v;
			setTopRow(qBound(qint64(0), newTopRow, scrollMaximum()));
		}
	}
}

void HexViewInternal::keyPressEvent(QKeyEvent *event)
{
	if (m_selecting || m_selectionStart == -1) {
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
		m_editor->seek(m_selectionStart);
		if (m_editor->atEnd()) {
			qint64 prevRowCount = rowCount();
			m_editor->insertByte(m_editingCellByte);
			if (prevRowCount != rowCount())
				emit rowCountChanged();
		} else {
			while (m_editor->position() <= m_selectionEnd) {
				m_editor->replaceByte(m_editingCellByte);
				m_editor->moveForward();
			}
		}
		m_editingCell = false;
	};

	static const QSet<int> movingKeys = {
		Qt::Key_Enter, Qt::Key_Tab,
		Qt::Key_Up, Qt::Key_Down, Qt::Key_Left, Qt::Key_Right,
	};

	if (m_selection == Selection::Cells) {
		if (keyIsHexDigit) {
			if (!m_editingCell) {
				m_editingCell = true;
				m_editingCellByte = char(QString(char(key)).toInt(nullptr, 16));
				repaint();
				return;
			} else {
				m_editingCellByte <<= 4;
				m_editingCellByte |= char(QString(char(key)).toInt(nullptr, 16));
				putByte();
				if (m_selectionStart == m_selectionEnd) {
					++m_selectionStart;
					++m_selectionEnd;
				}
				repaint();
				return;
			}
		} else if (key == Qt::Key_Escape) {
			if (m_editingCell) {
				m_editingCell = false;
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
				if (m_editor->atEnd()) {
					qint64 prevRowCount = rowCount();
					m_editor->insertByte(byte);
					if (prevRowCount != rowCount())
						emit rowCountChanged();
				} else {
					while (m_editor->position() <= m_selectionEnd) {
						m_editor->replaceByte(byte);
						m_editor->moveForward();
					}
				}
				if (m_selectionStart == m_selectionEnd) {
					++m_selectionStart;
					++m_selectionEnd;
				}
				repaint();
				return;
			}
		}
	}

	if (movingKeys.contains(key)) {
		if (m_editingCell)
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
		if (m_selectionStart == m_selectionEnd) {
			m_selectionStart = qBound(qint64(0), m_selectionStart + move, m_editor->size() - 1);
			m_selectionEnd = m_selectionStart;
			repaint();
			return;
		}
	}

	if (key == Qt::Key_Delete || key == Qt::Key_Backspace) {
		qint64 count = m_selectionEnd - m_selectionStart + 1;
		if (m_selectionEnd == m_editor->size())
			--count;
		for (qint64 i = 0; i < count; ++i) {
			qint64 prevRowCount = rowCount();
			m_editor->seek(m_selectionStart);
			m_editor->deleteByte();
			if (prevRowCount != rowCount())
				emit rowCountChanged();
		}
		m_selectionStart = m_selectionEnd = -1;
		m_selection = Selection::None;
		repaint();
		return;
	}

	QWidget::keyPressEvent(event);
}

qint64 HexViewInternal::getHoverCell(const QPoint &mousePos) const
{
	if (cursorIsInFindWidget(mousePos))
		return -1;

	qint64 x = mousePos.x();
	qint64 y = mousePos.y();

	x -= lineNumberWidth();

	{
		qint64 vx = x;
		while (vx > 8 * m_cellPadding + 8 * m_cellSize) {
			x -= m_cellPadding;
			vx -= m_cellPadding;
			vx -= 8 * m_cellPadding + 8 * m_cellSize;
		}
	}

	x -= m_cellPadding / 2;
	y -= m_cellPadding / 2;

	y += (m_cellPadding + m_cellSize) * m_topRow;

	qint64 xi = -1;
	qint64 yi = -1;

	if (x >= 0 && x < m_bytesPerLine * (m_cellPadding + m_cellSize))
		xi = x / (m_cellPadding + m_cellSize);

	if (y >= 0 && y <= m_editor->size() * (m_cellPadding + m_cellSize))
		yi = y / (m_cellPadding + m_cellSize);

	if (xi != -1 && yi != -1)
		return qMin(xi + m_bytesPerLine * yi, m_editor->size());

	return -1;
}

qint64 HexViewInternal::getHoverText(const QPoint &mousePos) const
{
	qint64 x = mousePos.x();
	qint64 y = mousePos.y();

	y += (m_cellPadding + m_cellSize) * m_topRow;

	x -= cellX(m_bytesPerLine + 1);

	qint64 xi = -1, yi = -2;
	if (x >= 0 && x < (m_characterWidth + 5) * m_bytesPerLine)
		xi = x / (m_characterWidth + 5);
	if (y >= 0 && y <= m_editor->size() * (m_cellPadding + m_cellSize))
		yi = y / (m_cellPadding + m_cellSize);

	if (xi != -1 && yi != -1)
		return qMin(xi + m_bytesPerLine * yi, m_editor->size());

	return -1;
}

int HexViewInternal::lineNumberDigitsCount() const
{
	if (m_editor == nullptr)
		return 0;

	int digits = 1;
	qint64 size = m_editor->size();
	while (size /= 16)
		++digits;
	return digits;
}

int HexViewInternal::lineNumberWidth() const
{
	return m_cellSize + lineNumberDigitsCount() * m_characterWidth;
}
