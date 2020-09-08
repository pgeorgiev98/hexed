#ifndef BYTEINPUTWIDGET_H
#define BYTEINPUTWIDGET_H

#include <QWidget>
#include <QVector>

class QLineEdit;
class QRadioButton;

class ByteInputWidget : public QWidget
{
	Q_OBJECT
public:
	explicit ByteInputWidget(QWidget *parent = nullptr);

public slots:
	void set(quint8 byte);
	bool inputValid() const;
	quint8 get() const;

signals:
	void validityChanged();

private slots:
	bool inputValid(int index) const;
	quint8 get(int index) const;
	void updateValues();

private:
	struct Base
	{
		QString name;
		QLineEdit *widget;
		int base;
	};

	const QVector<Base> m_bases;

	quint8 m_value;
	bool m_valid;
};

#endif // BYTEINPUTWIDGET_H
