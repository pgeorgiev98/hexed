#include "gotodialog.h"
#include "expressionvalidator.h"

#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QScriptEngine>
#include <QtMath>

static const QString emptyInputMessage = "No expression entered";
static const QString invalidExpressionMessage = "Invalid expression";
static const QString negativeValueMessage = "Address can't be negative";
static const QString addressTooLargeMessage = "Cannot input addresses larger than 2^52";
static const QString evaluationErrorMessage = "Evaluation error. Please report";
static const QString addressAfterEOFMessage = "Address can't be after the end of the file";

GotoDialog::GotoDialog(QWidget *parent)
	: QDialog(parent)
	, m_maximum(0)
	, m_position(0)
	, m_engine(new QScriptEngine(this))
	, m_inputField(new QLineEdit)
	, m_statusLabel(new QLabel)
	, m_goButton(new QPushButton("Go"))
{
	QVBoxLayout *layout = new QVBoxLayout;
	setLayout(layout);

	layout->addWidget(new QLabel(
						  "Enter the address you want to go to.\n"
						  "You can input any valid mathematical expression using +, -, *, /, (, ).\n"
						  "Prepend numbers with 0x for hex, with 0b for binary and with 0 for octal."));
	layout->addWidget(m_inputField);
	layout->addWidget(m_statusLabel);
	layout->addWidget(m_goButton);

	m_inputField->setValidator(new ExpressionValidator);
	connect(m_goButton, &QPushButton::clicked, this, &GotoDialog::go);
	connect(m_inputField, &QLineEdit::returnPressed, this, &GotoDialog::go);
	connect(m_inputField, &QLineEdit::textChanged, this, &GotoDialog::onInputChanged);

	onInputChanged();
}

qint64 GotoDialog::position() const
{
	return m_position;
}

void GotoDialog::setMaximum(qint64 maximum)
{
	m_maximum = maximum;
}

void GotoDialog::go()
{
	if (!m_goButton->isEnabled())
		return;

	accept();
}

void GotoDialog::onInputChanged()
{
	QString input = m_inputField->text();
	bool ok = true;

	if (input.isEmpty()) {
		m_statusLabel->setText(emptyInputMessage);
		ok = false;

	} else if (!m_inputField->hasAcceptableInput()) {
		m_statusLabel->setText(invalidExpressionMessage);
		ok = false;

	} else {
		input.remove(' ');

		// Convert the binary numbers to decimal as QScriptEngine doesn't seem to like them
		for (;;) {
			static const QRegExp binRegExp("0b[01]+");
			int i = binRegExp.indexIn(input);
			if (i == -1)
				break;
			int len = binRegExp.matchedLength();
			QString bin = input.mid(i + 2, len - 2);
			input.remove(i, len);
			input.insert(i, QString::number(bin.toLongLong(nullptr, 2)));
		}

		auto value = m_engine->evaluate(input);
		if (value.isNumber()) {
			if (value.toNumber() < 0.0) {
				m_statusLabel->setText(negativeValueMessage);
				ok = false;
			} else if (value.toNumber() > qPow(2, 52)) {
				m_statusLabel->setText(addressTooLargeMessage);
				ok = false;
			} else {
				bool isLongLong;
				m_position = value.toString().toLongLong(&isLongLong);
				if (!isLongLong) {
					m_statusLabel->setText(evaluationErrorMessage);
					ok = false;
				} else if (m_position > m_maximum) {
					m_statusLabel->setText(addressAfterEOFMessage);
					ok = false;
				}
			}
		}
	}

	if (ok) {
		auto putSpaces = [](const QString &in, int place) -> QString {
			QString out;
			for (int i = 0; i < in.size(); ++i) {
				if (i % place == 0 && i != 0)
					out.prepend(' ');
				out.prepend(in[in.size() - i - 1]);
			}
			return out;
		};
		m_statusLabel->setText(QString("Hex: %1\nDec: %2\nBin: %3").
							   arg(putSpaces(QString::number(m_position, 16).toUpper(), 4)).
							   arg(putSpaces(QString::number(m_position, 10), 3)).
							   arg(putSpaces(QString::number(m_position, 2), 8)));
		m_statusLabel->setStyleSheet("");
	} else {
		m_statusLabel->setStyleSheet("color: red");
	}


	m_goButton->setEnabled(ok);
}
