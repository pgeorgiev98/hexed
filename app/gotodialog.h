#ifndef GOTODIALOG_H
#define GOTODIALOG_H

#include <QDialog>

class QScriptEngine;
class QLineEdit;
class QPushButton;
class QLabel;

class GotoDialog : public QDialog
{
	Q_OBJECT
public:
	explicit GotoDialog(QWidget *parent = nullptr);
	qint64 position() const;
	void setMaximum(qint64 maximum);

private slots:
	void go();
	void onInputChanged();

private:
	qint64 m_maximum;
	qint64 m_position;

	QScriptEngine *m_engine;

	QLineEdit *m_inputField;
	QLabel *m_statusLabel;
	QPushButton *m_goButton;
};

#endif // GOTODIALOG_H
