#include "hexview.h"
#include "hexviewinternal.h"

#include <QHBoxLayout>
#include <QScrollBar>

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
	connect(m_hexViewInternal, &HexViewInternal::rowCountChanged, this, &HexView::onRowCountChanged);
	connect(m_hexViewInternal, &HexViewInternal::topRowChanged, this, &HexView::onTopRowChanged);
	connect(m_verticalScrollBar, &QScrollBar::valueChanged, this, &HexView::onScrollBarChanged);
}

void HexView::onRowCountChanged()
{
	// TODO
	qint64 rowCount = m_hexViewInternal->rowCount();
	m_verticalScrollBar->setMaximum(int(rowCount) - 1);
}

void HexView::onTopRowChanged(int topRow)
{
	// TODO
	m_verticalScrollBar->setValue(topRow);
}

void HexView::onScrollBarChanged(int value)
{
	// TODO
	m_hexViewInternal->setTopRow(value);
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
