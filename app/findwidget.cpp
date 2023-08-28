#include "findwidget.h"
#include "bufferededitor.h"
#include "hexviewinternal.h"
#include "finder.h"
#include "iconprovider.h"

#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

#include <QKeyEvent>
#include <QRegExpValidator>
#include <QFontMetrics>

static int textWidth(QFontMetrics fm, const QString &text)
{
#if QT_VERSION >= 0x050B00
	return fm.horizontalAdvance(text);
#else
	return fm.width(text);
#endif
}

FindWidget::FindWidget(HexViewInternal *hexView, QWidget *parent)
	: QWidget(parent)
	, m_hexView(hexView)
	, m_finder(new Finder(m_hexView->editor(), this))
	, m_selectionChanged(false)
	, m_input(new QLineEdit)
	, m_message(new QLabel)
{
	setAutoFillBackground(true);
	// TODO: Automatically place a space here
	m_input->setValidator(new QRegExpValidator(QRegExp("([0-9a-fA-f]{2} )*([0-9a-fA-f]{2})")));

	m_message->setFixedWidth(1.2 * textWidth(QFontMetrics(m_message->font()), "Search reached end of file"));
	m_message->setAlignment(Qt::AlignCenter);

	QPushButton *up = new QPushButton;
	QPushButton *down = new QPushButton;
	QPushButton *close = new QPushButton;
	up->setIcon(IconProvider::getContrastingIcon(IconProvider::upArrow, up));
	up->setDisabled(true);
	down->setIcon(IconProvider::getContrastingIcon(IconProvider::downArrow, down));
	down->setDisabled(true);
	close->setIcon(IconProvider::getContrastingIcon(IconProvider::cross, close));

	QHBoxLayout *layout = new QHBoxLayout;
	setLayout(layout);

	layout->addWidget(m_input);
	layout->addWidget(up);
	layout->addWidget(down);
	layout->addWidget(m_message);
	layout->addWidget(close, 0, Qt::AlignRight);

	connect(m_hexView, &HexViewInternal::userChangedSelection, [this]() { m_selectionChanged = true; });

	connect(close, &QPushButton::clicked, this, &FindWidget::close);
	connect(m_input, &QLineEdit::textChanged, [this, up, down]() {
		bool ok = m_input->hasAcceptableInput();
		up->setEnabled(ok);
		down->setEnabled(ok);
	});

	connect(down, &QPushButton::clicked, this, &FindWidget::searchDown);
	connect(m_input, &QLineEdit::returnPressed, this, &FindWidget::searchDown);
}

void FindWidget::close()
{
	m_message->clear();
	hide();
	emit closed();
}

QByteArray FindWidget::searchData() const
{
	QByteArray arr;
	QStringList parts = m_input->text().split(' ', Qt::SkipEmptyParts);
	for (const QString &s : parts)
		arr.append(s.toInt(nullptr, 16));
	return arr;
}

void FindWidget::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Escape) {
		close();
	} else {
		QWidget::keyPressEvent(event);
	}
}

void FindWidget::focusInEvent(QFocusEvent *)
{
	m_input->setFocus();
	m_input->selectAll();
}

void FindWidget::showEvent(QShowEvent *)
{
	m_input->setPlaceholderText("DE 3E 99 0B F5 ...");
}

void FindWidget::searchDown()
{
	QByteArray sd = searchData();
	if (m_finder->searchData() != sd || m_selectionChanged) {
		qint64 position;
		auto selection = m_hexView->selection();
		if (selection)
			position = selection->begin + selection->count;
		else
			position = m_hexView->m_topRow * m_hexView->m_bytesPerLine;
		m_finder->search(position, sd);
		m_selectionChanged = false;
	}

	// TODO: Make this asynchronous
	m_finder->findNext();

	qint64 result = m_finder->searchResultPosition();
	if (result != -1) {
		m_message->clear();
		// TODO: Scroll to the result properly
		m_hexView->setTopRow(result / m_hexView->bytesPerLine());
		m_hexView->highlight(ByteSelection(result, sd.size()));
	} else {
		m_message->setText("Search reached end of file");
	}
}
