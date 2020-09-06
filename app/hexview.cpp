#include "hexview.h"
#include "hexviewinternal.h"
#include "bufferededitor.h"
#include "common.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QScrollBar>
#include <QStatusBar>

// TODO: Optionally don't scroll in real time while the scrollbar is being dragged

HexView::HexView(QWidget *parent)
	: QWidget(parent)
	, m_hexViewInternal(new HexViewInternal)
	, m_verticalScrollBar(new QScrollBar)
	, m_statusBar(new QStatusBar)
	, m_fileSizeLabel(new QLabel)
	, m_selectionLabel(new QLabel)
{
	m_statusBar->addPermanentWidget(m_selectionLabel);
	m_statusBar->addPermanentWidget(m_fileSizeLabel);

	QHBoxLayout *hbox = new QHBoxLayout;
	hbox->setContentsMargins(0, 0, 0, 0);
	hbox->addWidget(m_hexViewInternal);
	hbox->addWidget(m_verticalScrollBar, 0, Qt::AlignRight);

	QVBoxLayout *vbox = new QVBoxLayout;
	vbox->setContentsMargins(0, 0, 0, 0);
	vbox->addLayout(hbox, 1);
	vbox->addWidget(m_statusBar, 0, Qt::AlignBottom);

	setLayout(vbox);

	connect(m_hexViewInternal, &HexViewInternal::canUndoChanged, this, &HexView::canUndoChanged);
	connect(m_hexViewInternal, &HexViewInternal::canRedoChanged, this, &HexView::canRedoChanged);
	connect(m_hexViewInternal, &HexViewInternal::rowCountChanged, this, &HexView::updateScrollMaximum);
	connect(m_hexViewInternal, &HexViewInternal::topRowChanged, this, &HexView::setTopRow);
	connect(m_hexViewInternal, &HexViewInternal::scrollMaximumChanged, this, &HexView::updateScrollMaximum);
	connect(m_hexViewInternal, &HexViewInternal::selectionChanged, this, &HexView::selectionChanged);
	connect(m_verticalScrollBar, &QScrollBar::valueChanged, this, &HexView::onScrollBarChanged);
}

std::optional<ByteSelection> HexView::selection() const
{
	return m_hexViewInternal->selection();
}

void HexView::updateScrollMaximum()
{
	qint64 scrollMaximum = m_hexViewInternal->scrollMaximum();
	m_verticalScrollBar->setMaximum(scrollMaximum / scrollStep(scrollMaximum) + 1);
}

void HexView::setTopRow(qint64 topRow)
{
	qint64 scrollMaximum = m_hexViewInternal->scrollMaximum();
	m_verticalScrollBar->setValue(topRow / scrollStep(scrollMaximum));
}

void HexView::onScrollBarChanged(int value)
{
	qint64 scrollMaximum = m_hexViewInternal->scrollMaximum();
	qint64 topRow = qint64(value) * scrollStep(scrollMaximum);
	if (topRow > scrollMaximum)
		topRow = scrollMaximum;
	m_hexViewInternal->setTopRow(topRow);
}

void HexView::updateStatusBar()
{
	// File size
	qint64 fileSize = m_hexViewInternal->editor()->size();
	QString fileSizeString;
	fileSizeString.append(QString("%1B").arg(fileSize));
	if (fileSize >= 1024)
		fileSizeString.append(QString(" (%1)").arg(prettySize(fileSize)));
	m_fileSizeLabel->setText(fileSizeString);

	// Selection
	auto selection = m_hexViewInternal->selection();
	QString selectionText;
	if (selection) {
		ByteSelection sel = *selection;
		if (sel.count == 1)
			selectionText = QString("Selected 0x%1").arg(sel.begin, m_hexViewInternal->lineNumberDigitsCount(), 16, QChar('0'));
		else if (sel.count > 1)
			selectionText = QString("Selected %1 bytes (0x%2 to 0x%3)")
					.arg(sel.count)
					.arg(sel.begin, m_hexViewInternal->lineNumberDigitsCount(), 16, QChar('0'))
					.arg(sel.begin + sel.count - 1, m_hexViewInternal->lineNumberDigitsCount(), 16, QChar('0'));
	}
	m_selectionLabel->setText(selectionText);
}

int HexView::scrollStep(qint64 rowCount) const
{
	const qint64 maxScrollValue = 2100000000;
	int step = rowCount / maxScrollValue;
	step += (rowCount % maxScrollValue > 0);
	return qMax(step, 1);
}


bool HexView::canUndo() const
{
	return m_hexViewInternal->canUndo();
}

bool HexView::canRedo() const
{
	return m_hexViewInternal->canRedo();
}

bool HexView::openFile(const QString &path)
{
	bool result = m_hexViewInternal->openFile(path);
	BufferedEditor *editor = m_hexViewInternal->editor();
	connect(editor, &BufferedEditor::sizeChanged, this, &HexView::updateStatusBar);
	connect(m_hexViewInternal, &HexViewInternal::selectionChanged, this, &HexView::updateStatusBar);
	updateStatusBar();
	return result;
}

bool HexView::saveChanges()
{
	return m_hexViewInternal->saveChanges();
}

bool HexView::quit()
{
	return m_hexViewInternal->quit();
}

void HexView::undo()
{
	m_hexViewInternal->undo();
}

void HexView::redo()
{
	m_hexViewInternal->redo();
}

void HexView::selectAll()
{
	m_hexViewInternal->selectAll();
}

void HexView::selectNone()
{
	m_hexViewInternal->selectNone();
}

void HexView::copyText()
{
	ByteSelection selection = *m_hexViewInternal->selection();
	selection.type = ByteSelection::Type::Text;
	m_hexViewInternal->copy(selection);
}

void HexView::copyHex()
{
	ByteSelection selection = *m_hexViewInternal->selection();
	selection.type = ByteSelection::Type::Cells;
	m_hexViewInternal->copy(selection);
}

void HexView::openGotoDialog()
{
	m_hexViewInternal->openGotoDialog();
}

void HexView::openFindDialog()
{
	m_hexViewInternal->openFindDialog();
}
