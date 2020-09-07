#include "baseconverter.h"
#include "endianconverter.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QKeyEvent>
#include <QRadioButton>
#include <QSettings>

static const QString settingsEndianKey = "baseconverter_endianness";

BaseConverter::BaseConverter(QWidget *parent)
	: QWidget(parent)
	, m_bases({
			  Base{"Bin", new QLineEdit, 2},
			  Base{"Oct", new QLineEdit, 8},
			  Base{"Dec", new QLineEdit, 10},
			  Base{"Hex", new QLineEdit, 16},
			  })
	, m_littleEndianButton(new QRadioButton("Little Endian"))
	, m_bigEndianButton(new QRadioButton("Big Endian"))
	, m_value(0)
{
	setWindowFlag(Qt::WindowType::Dialog);

	// Load saved settings
	QSettings settings;
	if ("big" == settings.value(settingsEndianKey).toString())
		m_endian = Endian::Big;
	else
		m_endian = Endian::Little;

	// The main widget layout
	QGridLayout *layout = new QGridLayout;
	setLayout(layout);
	int row = 0;

	// Endianness radio buttons
	{
		QHBoxLayout *l = new QHBoxLayout;
		l->addWidget(m_littleEndianButton);
		l->addWidget(m_bigEndianButton);
		layout->addLayout(l, row++, 0, 1, 2);
		m_littleEndianButton->setChecked(m_endian == Endian::Little);
		m_bigEndianButton->setChecked(m_endian == Endian::Big);
		connect(m_littleEndianButton, &QRadioButton::toggled, this, &BaseConverter::onLittleEndianToggled);
		connect(m_bigEndianButton, &QRadioButton::toggled, this, &BaseConverter::onBigEndianToggled);
	}

	// The value input fields
	for (const Base &base : m_bases) {
		layout->addWidget(new QLabel(base.name), row, 0);
		layout->addWidget(base.widget, row++, 1);
		base.widget->setAlignment(Qt::AlignRight);
		connect(base.widget, &QLineEdit::textEdited, this, &BaseConverter::updateValues);
	}
}

void BaseConverter::setFromBytes(const QByteArray &bytes)
{
	if (m_endian == Endian::Little)
		setValue(EndianConverter::littleEndianToNumber(bytes));
	else
		setValue(EndianConverter::bigEndianToNumber(bytes));
}

void BaseConverter::updateValues()
{
	QLineEdit *w = qobject_cast<QLineEdit *>(sender());
	if (w == nullptr)
		return;

	int base = 0;
	for (Base b : m_bases) {
		if (b.widget == w) {
			base = b.base;
			break;
		}
	}

	Q_ASSERT(base);
	if (base == 0)
		return;

	bool ok;
	m_value = w->text().replace(" ", "").toLongLong(&ok, base);
	for (Base b : m_bases)
		if (b.widget != w)
			b.widget->setText(ok ? QString::number(m_value, b.base).toUpper() : QString());
}

void BaseConverter::setValue(quint64 value)
{
	m_value = value;
	for (Base b : m_bases)
		b.widget->setText(QString::number(value, b.base).toUpper());
}

void BaseConverter::onLittleEndianToggled()
{
	if (m_littleEndianButton->isChecked()) {
		QSettings settings;
		m_endian = Endian::Little;
		settings.setValue(settingsEndianKey, "little");
		setValue(EndianConverter::littleEndianToNumber(EndianConverter::bigEndianToBytes(m_value, calcValueByteCount(m_value))));
	}
}

void BaseConverter::onBigEndianToggled()
{
	if (m_bigEndianButton->isChecked()) {
		QSettings settings;
		m_endian = Endian::Big;
		settings.setValue(settingsEndianKey, "big");
		setValue(EndianConverter::bigEndianToNumber(EndianConverter::littleEndianToBytes(m_value, calcValueByteCount(m_value))));
	}
}

void BaseConverter::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Escape) {
		event->accept();
		hide();
	}
}

quint8 BaseConverter::calcValueByteCount(quint64 value)
{
	quint8 c = 0;
	do {
		value /= 0x100;
		++c;
	} while (value);

	return c;
}
