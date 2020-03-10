#ifndef FINDWIDGET_H
#define FINDWIDGET_H

#include <QWidget>

class HexViewInternal;
class Finder;

class QLineEdit;
class QLabel;

class FindWidget : public QWidget
{
	Q_OBJECT
public:
	explicit FindWidget(HexViewInternal *hexView, QWidget *parent = nullptr);

signals:
	void closed();

public slots:
	void close();
	QByteArray searchData() const;

protected:
	void keyPressEvent(QKeyEvent *) override;
	void focusInEvent(QFocusEvent *) override;

private slots:
	void searchDown();

private:
	HexViewInternal *m_hexView;
	Finder *m_finder;

	QLineEdit *m_input;
	QLabel *m_message;
};

#endif // FINDWIDGET_H
