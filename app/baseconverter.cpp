#include "baseconverter.h"
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QKeyEvent>

BaseConverter::BaseConverter(QWidget *parent)
	: QWidget(parent)
	, m_bin(new QLineEdit)
	, m_oct(new QLineEdit)
	, m_dec(new QLineEdit)
	, m_hex(new QLineEdit)
{
	setWindowFlag(Qt::WindowType::Dialog);

	QGridLayout *layout = new QGridLayout;
	setLayout(layout);

	int row = 0;
	layout->addWidget(new QLabel("Binary: "), row, 0);
	layout->addWidget(m_bin, row++, 1);
	layout->addWidget(new QLabel("Octal: "), row, 0);
	layout->addWidget(m_oct, row++, 1);
	layout->addWidget(new QLabel("Decimal: "), row, 0);
	layout->addWidget(m_dec, row++, 1);
	layout->addWidget(new QLabel("Hexadecimal: "), row, 0);
	layout->addWidget(m_hex, row++, 1);

	for (QLineEdit *e : {m_bin, m_oct, m_dec, m_hex}) {
		e->setAlignment(Qt::AlignRight);
		connect(e, &QLineEdit::textEdited, this, &BaseConverter::updateValues);
	}
}

void BaseConverter::updateValues()
{
	QLineEdit *w = qobject_cast<QLineEdit *>(sender());
	if (w == nullptr)
		return;

	struct Base
	{
		QLineEdit *widget;
		int base;
	};
	QVector<Base> bases = {
		{m_bin, 2},
		{m_oct, 8},
		{m_dec, 10},
		{m_hex, 16},
	};

	int base = 0;
	for (Base b : bases) {
		if (b.widget == w) {
			base = b.base;
			break;
		}
	}

	Q_ASSERT(base);
	if (base == 0)
		return;

	bool ok;
	qint64 value = w->text().replace(" ", "").toLongLong(&ok, base);
	for (Base b : bases)
		if (b.widget != w)
			b.widget->setText(ok ? QString::number(value, b.base) : QString());
}

void BaseConverter::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Escape) {
		event->accept();
		hide();
	}
}
