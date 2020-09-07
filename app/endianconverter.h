#ifndef BYTESTONUMBER_H
#define BYTESTONUMBER_H

#include <QByteArray>

class EndianConverter
{
public:
	static quint64 littleEndianToNumber(const QByteArray &bytes);
	static quint64 bigEndianToNumber(const QByteArray &bytes);
	static QByteArray littleEndianToBytes(quint64 value, quint8 byteCount);
	static QByteArray bigEndianToBytes(quint64 value, quint8 byteCount);
};

#endif // BYTESTONUMBER_H
