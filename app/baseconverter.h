#ifndef BASECONVERTER_H
#define BASECONVERTER_H

#include <QWidget>
#include <QVector>

class QLineEdit;
class QRadioButton;

class BaseConverter : public QWidget
{
	Q_OBJECT
public:
	explicit BaseConverter(QWidget *parent = nullptr);

public slots:
	void setFromBytes(const QByteArray &bytes);

private slots:
	void updateValues();
	void setValue(quint64 value);
	void onLittleEndianToggled();
	void onBigEndianToggled();

protected:
	void keyPressEvent(QKeyEvent *event) override;

private:
	static quint8 calcValueByteCount(quint64 value);

	struct Base
	{
		QString name;
		QLineEdit *widget;
		int base;
	};

	enum Endian
	{
		Big, Little,
	};

	const QVector<Base> m_bases;

	QRadioButton *m_littleEndianButton;
	QRadioButton *m_bigEndianButton;

	Endian m_endian;
	quint64 m_value;
};

#endif // BASECONVERTER_H
