#include "expressionvalidator.h"

#include <QString>
#include <QRegExp>
#include <QRegExpValidator>

static const QString numberRegExp = "(\\(|)*(0x[0-9A-Fa-f][0-9A-Fa-f ]*|0b[01][01 ]*|0[0-7][0-7 ]*|[1-9][0-9 ]*|0)(\\))*";
static const QString expressionRegExp = QString("%1([+\\-*/]%2)*").arg(numberRegExp).arg(numberRegExp);

ExpressionValidator::ExpressionValidator(QObject *parent)
	: QValidator(parent)
	, m_regExpValidator(new QRegExpValidator(QRegExp(expressionRegExp), this))
{
}

QValidator::State ExpressionValidator::validate(QString &input, int &pos) const
{
	auto state = m_regExpValidator->validate(input, pos);
	if (state == QValidator::Acceptable) {
		// Check matching brackets
		int brackets = 0;
		for (QChar b : input) {
			if (b == '(') {
				++brackets;
			} else if (b == ')') {
				--brackets;
				if (brackets < 0)
					return QValidator::Intermediate;
			}
		}
		if (brackets != 0)
			return QValidator::Intermediate;
	}
	return state;
}
