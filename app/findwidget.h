#ifndef FINDWIDGET_H
#define FINDWIDGET_H

#include <QWidget>

class QLineEdit;

class FindWidget : public QWidget
{
	Q_OBJECT
public:
	explicit FindWidget(QWidget *parent = nullptr);

signals:
	void closed();

public slots:
	void close();

protected:
	void keyPressEvent(QKeyEvent *) override;
	void focusInEvent(QFocusEvent *) override;

private:
	QLineEdit *m_input;
};

#endif // FINDWIDGET_H
