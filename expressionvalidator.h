#ifndef EXPRESSIONVALIDATOR_H
#define EXPRESSIONVALIDATOR_H

#include <QValidator>

class QRegExpValidator;

class ExpressionValidator : public QValidator
{
	Q_OBJECT
public:
	explicit ExpressionValidator(QObject *parent = nullptr);

protected:
	QValidator::State validate(QString &input, int &pos) const override;

private:
	QRegExpValidator *m_regExpValidator;
};

#endif // EXPRESSIONVALIDATOR_H
