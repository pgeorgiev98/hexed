#include "byteinputwidget.h"

#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>

ByteInputWidget::ByteInputWidget(QWidget *parent)
	: QWidget(parent)
	, m_bases({
			  Base{"Bin", new QLineEdit, 2},
			  Base{"Oct", new QLineEdit, 8},
			  Base{"Dec", new QLineEdit, 10},
			  Base{"Hex", new QLineEdit, 16},
			  })
	, m_value(0)
	, m_valid(false)
{
	// The main widget layout
	QGridLayout *layout = new QGridLayout;
	setLayout(layout);
	int row = 0;

	// The value input fields
	for (const Base &base : m_bases) {
		layout->addWidget(new QLabel(base.name), row, 0);
		layout->addWidget(base.widget, row++, 1);
		base.widget->setAlignment(Qt::AlignRight);
		connect(base.widget, &QLineEdit::textEdited, this, &ByteInputWidget::updateValues);
	}
}

void ByteInputWidget::set(quint8 byte)
{
	m_value = byte;
	for (Base b : m_bases)
		b.widget->setText(QString::number(byte, b.base).toUpper());
}

quint8 ByteInputWidget::get() const
{
	return m_value;
}

bool ByteInputWidget::inputValid() const
{
	return m_valid;
}

bool ByteInputWidget::inputValid(int index) const
{
	bool valid = !m_bases[index].widget->text().isEmpty();

	uint value;
	if (valid)
		value = m_bases[index].widget->text().replace(" ", "").toUInt(&valid, m_bases[index].base);

	if (valid)
		valid = (value < 256);

	return valid;
}

quint8 ByteInputWidget::get(int index) const
{
	Q_ASSERT(inputValid(index));
	uint value = m_bases[index].widget->text().replace(" ", "").toUInt(nullptr, m_bases[index].base);
	return quint8(value);
}

void ByteInputWidget::updateValues()
{
	QLineEdit *w = qobject_cast<QLineEdit *>(sender());
	if (w == nullptr)
		return;

	int index = -1;
	for (int i = 0; i < m_bases.size(); ++i) {
		if (m_bases[i].widget == w) {
			index = i;
			break;
		}
	}

	Q_ASSERT(index >= 0);
	if (index < 0)
		return;

	bool oldValid = m_valid;
	m_valid = inputValid(index);
	if (m_valid)
		m_value = get(index);

	for (Base b : m_bases)
		if (b.widget != w)
			b.widget->setText(m_valid ? QString::number(m_value, b.base).toUpper() : QString());

	if (m_valid != oldValid)
		emit validityChanged();
}
