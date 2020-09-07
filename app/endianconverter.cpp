#include "endianconverter.h"

quint64 EndianConverter::littleEndianToNumber(const QByteArray &bytes)
{
	Q_ASSERT(bytes.size() <= 8);
	quint64 value = 0;
	for (int i = 0; i < bytes.size(); ++i)
		value += (quint64(quint8(bytes[i])) << (i * 8));
	return value;
}

quint64 EndianConverter::bigEndianToNumber(const QByteArray &bytes)
{
	Q_ASSERT(bytes.size() <= 8);
	quint64 value = 0;
	for (int i = 0; i < bytes.size(); ++i)
		value += (quint64(quint8(bytes[i])) << ((bytes.size() - i - 1) * 8));
	return value;
}

QByteArray EndianConverter::littleEndianToBytes(quint64 value, quint8 byteCount)
{
	Q_ASSERT(byteCount <= 8 && byteCount >= 1);
	QByteArray bytes;
	for (quint64 i = 0; i < byteCount; ++i)
		bytes.append(quint8((value & (quint64(0xFF) << (i * 8))) >> (i * 8)));
	return bytes;
}

QByteArray EndianConverter::bigEndianToBytes(quint64 value, quint8 byteCount)
{
	Q_ASSERT(byteCount <= 8 && byteCount >= 1);
	QByteArray bytes;
	for (quint64 i = byteCount; i-- > 0;)
		bytes.append(quint8((value & (quint64(0xFF) << (i * 8))) >> (i * 8)));
	return bytes;
}
