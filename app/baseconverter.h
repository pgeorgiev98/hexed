#ifndef BASECONVERTER_H
#define BASECONVERTER_H

#include <QWidget>

class QLineEdit;

class BaseConverter : public QWidget
{
	Q_OBJECT
public:
	explicit BaseConverter(QWidget *parent = nullptr);

private slots:
	void updateValues();

protected:
	void keyPressEvent(QKeyEvent *event) override;

private:
	QLineEdit *m_bin;
	QLineEdit *m_oct;
	QLineEdit *m_dec;
	QLineEdit *m_hex;
};

#endif // BASECONVERTER_H
