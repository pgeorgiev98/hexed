#include "hexview.h"
#include "hexviewinternal.h"
#include "bufferededitor.h"
#include "endianconverter.h"
#include "common.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QScrollBar>
#include <QStatusBar>

// TODO: Optionally don't scroll in real time while the scrollbar is being dragged

HexView::HexView(QWidget *parent)
	: QWidget(parent)
	, m_hexViewsLayout(new QHBoxLayout)
	, m_verticalScrollBar(new QScrollBar)
	, m_statusBar(new QStatusBar)
	, m_fileSizeLabel(new QLabel)
	, m_selectionLabel(new QLabel)
{
	m_statusBar->addPermanentWidget(m_selectionLabel, 1);
	m_statusBar->addPermanentWidget(m_fileSizeLabel);

	QHBoxLayout *hbox = new QHBoxLayout;
	hbox->setContentsMargins(0, 0, 0, 0);
	hbox->addLayout(m_hexViewsLayout);
	hbox->addWidget(m_verticalScrollBar, 0, Qt::AlignRight);

	QVBoxLayout *vbox = new QVBoxLayout;
	vbox->setContentsMargins(0, 0, 0, 0);
	vbox->addLayout(hbox, 1);
	vbox->addWidget(m_statusBar, 0, Qt::AlignBottom);

	setLayout(vbox);

	connect(m_verticalScrollBar, &QScrollBar::valueChanged, this, &HexView::onScrollBarChanged);
}

std::optional<ByteSelection> HexView::selection() const
{
	// TODO
	return std::optional<ByteSelection>();
	//return m_hexViewInternal->selection();
}

void HexView::updateScrollMaximum()
{
	qint64 scrollMax = scrollMaximum();
	m_verticalScrollBar->setMaximum(scrollMax / scrollStep(scrollMax) + 1);
}

void HexView::setTopRow(qint64 topRow)
{
	m_verticalScrollBar->setValue(topRow / scrollStep(scrollMaximum()));
}

void HexView::onScrollBarChanged(int value)
{
	qint64 scrollMax = scrollMaximum();
	qint64 topRow = qint64(value) * scrollStep(scrollMax);
	if (topRow > scrollMax)
		topRow = scrollMax;
	for (auto h : m_hexViews)
		h->setTopRow(topRow);
}

void HexView::updateStatusBar()
{
	// TODO
	/*
	// File size
	qint64 fileSize = m_hexViewInternal->editor()->size();
	QString fileSizeString = "Total: ";
	fileSizeString.append(QString("%1B").arg(fileSize));
	if (fileSize >= 1024)
		fileSizeString.append(QString(" (%1)").arg(prettySize(fileSize)));
	m_fileSizeLabel->setText(fileSizeString);

	// Selection
	auto selection = m_hexViewInternal->selection();
	QString selectionText;
	if (selection) {
		ByteSelection sel = *selection;

		QByteArray bytes;
		auto editor = m_hexViewInternal->editor();
		if (sel.count <= 4 && sel.begin != editor->size()) {
			editor->seek(sel.begin);
			for (int i = 0; i < sel.count; ++i)
				bytes.append(quint8(*(editor->getByte().current)));
		}

		if (sel.begin == editor->size()) {
			selectionText = QString("Selected 0x%1")
					.arg(sel.begin, m_hexViewInternal->lineNumberDigitsCount(), 16, QChar('0'));

		} else if (sel.count == 1) {
			selectionText = QString("Selected 0x%1, Dec: %2")
					.arg(sel.begin, m_hexViewInternal->lineNumberDigitsCount(), 16, QChar('0'))
					.arg(quint64(quint8(bytes[0])));

		} else if (sel.count > 1) {
			selectionText = QString("Selected %1 bytes (0x%2 to 0x%3)")
					.arg(sel.count)
					.arg(sel.begin, m_hexViewInternal->lineNumberDigitsCount(), 16, QChar('0'))
					.arg(sel.begin + sel.count - 1, m_hexViewInternal->lineNumberDigitsCount(), 16, QChar('0'));

			if (!bytes.isEmpty()) {
				quint64 le = EndianConverter::littleEndianToNumber(bytes);
				quint64 be = EndianConverter::bigEndianToNumber(bytes);
				selectionText.append(QString(", LE: %1, BE: %2").arg(le).arg(be));
			}

		}
	}
	m_selectionLabel->setText(selectionText);
	*/
}

void HexView::onUserChangedSelection()
{
	// TODO: Test different sizes
	// TODO: And selectAll
	HexViewInternal *hexView = qobject_cast<HexViewInternal *>(sender());
	Q_ASSERT(hexView);
	auto selection = hexView->selection();
	for (auto h : m_hexViews) {
		if (h != hexView) {
			if (selection)
				h->highlight(*selection);
			else
				h->selectNone();
		}
	}
}

int HexView::scrollStep(qint64 rowCount) const
{
	const qint64 maxScrollValue = 2100000000;
	int step = rowCount / maxScrollValue;
	step += (rowCount % maxScrollValue > 0);
	return qMax(step, 1);
}

qint64 HexView::scrollMaximum() const
{
	qint64 scrollMaximum = 0;
	for (auto h : m_hexViews)
		scrollMaximum = qMax(scrollMaximum, h->scrollMaximum());
	return scrollMaximum;
}


bool HexView::canUndo() const
{
	// TODO
	//return m_hexViewInternal->canUndo();
	return false;
}

bool HexView::canRedo() const
{
	// TODO
	//return m_hexViewInternal->canRedo();
	return false;
}

BufferedEditor *HexView::editor()
{
	// TODO
	//return m_hexViewInternal->editor();
	return nullptr;
}

bool HexView::openFile(const QString &path)
{
	HexViewInternal *hexView = new HexViewInternal;
	bool result = hexView->openFile(path);

	if (result) {
		m_hexViewsLayout->addWidget(hexView);
		m_hexViews.append(hexView);
		for (auto h : m_hexViews)
			h->setDiffGroup(m_hexViews);

		BufferedEditor *editor = hexView->editor();
		connect(editor, &BufferedEditor::sizeChanged, this, &HexView::updateStatusBar);
		connect(hexView, &HexViewInternal::selectionChanged, this, &HexView::updateStatusBar);
		connect(hexView, &HexViewInternal::canUndoChanged, this, &HexView::canUndoChanged);
		connect(hexView, &HexViewInternal::canRedoChanged, this, &HexView::canRedoChanged);
		connect(hexView, &HexViewInternal::rowCountChanged, this, &HexView::updateScrollMaximum);
		connect(hexView, &HexViewInternal::topRowChanged, this, &HexView::setTopRow);
		connect(hexView, &HexViewInternal::scrollMaximumChanged, this, &HexView::updateScrollMaximum);
		connect(hexView, &HexViewInternal::selectionChanged, this, &HexView::selectionChanged); // TODO: Deduplicate
		connect(hexView, &HexViewInternal::userChangedSelection, this, &HexView::onUserChangedSelection);
		updateStatusBar();
	} else {
		hexView->deleteLater();
	}

	return result;
}

bool HexView::saveChanges()
{
	// TODO
	return false;
	//return m_hexViewInternal->saveChanges();
}

bool HexView::quit()
{
	// TODO
	bool ok = true;
	for (auto h : m_hexViews)
		ok &= h->quit();
	return ok;
}

void HexView::undo()
{
	// TODO
	//m_hexViewInternal->undo();
}

void HexView::redo()
{
	// TODO
	//m_hexViewInternal->redo();
}

void HexView::selectAll()
{
	// TODO
	//m_hexViewInternal->selectAll();
}

void HexView::selectNone()
{
	// TODO
	//m_hexViewInternal->selectNone();
}

void HexView::copyText()
{
	// TODO
	//ByteSelection selection = *m_hexViewInternal->selection();
	//selection.type = ByteSelection::Type::Text;
	//m_hexViewInternal->copy(selection);
}

void HexView::copyHex()
{
	// TODO
	//ByteSelection selection = *m_hexViewInternal->selection();
	//selection.type = ByteSelection::Type::Cells;
	//m_hexViewInternal->copy(selection);
}

void HexView::openGotoDialog()
{
	// TODO
	//m_hexViewInternal->openGotoDialog();
}

void HexView::openFindDialog()
{
	// TODO
	//m_hexViewInternal->openFindDialog();
}
