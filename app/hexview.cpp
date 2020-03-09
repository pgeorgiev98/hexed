#include "hexview.h"
#include "hexviewinternal.h"

#include <QHBoxLayout>
#include <QScrollBar>

// TODO: Optionally don't scroll in real time while the scrollbar is being dragged

HexView::HexView(QWidget *parent)
	: QWidget(parent)
	, m_hexViewInternal(new HexViewInternal)
	, m_verticalScrollBar(new QScrollBar)
{
	QHBoxLayout *hbox = new QHBoxLayout;
	hbox->addWidget(m_hexViewInternal);
	hbox->addWidget(m_verticalScrollBar, 0, Qt::AlignRight);
	hbox->setContentsMargins(0, 0, 0, 0);

	setLayout(hbox);

	connect(m_hexViewInternal, &HexViewInternal::canUndoChanged, this, &HexView::canUndoChanged);
	connect(m_hexViewInternal, &HexViewInternal::canRedoChanged, this, &HexView::canRedoChanged);
	connect(m_hexViewInternal, &HexViewInternal::rowCountChanged, this, &HexView::updateScrollMaximum);
	connect(m_hexViewInternal, &HexViewInternal::topRowChanged, this, &HexView::setTopRow);
	connect(m_hexViewInternal, &HexViewInternal::scrollMaximumChanged, this, &HexView::updateScrollMaximum);
	connect(m_verticalScrollBar, &QScrollBar::valueChanged, this, &HexView::onScrollBarChanged);
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
	return m_hexViewInternal->openFile(path);
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

void HexView::openGotoDialog()
{
	m_hexViewInternal->openGotoDialog();
}
