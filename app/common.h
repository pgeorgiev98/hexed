#ifndef COMMON_H
#define COMMON_H

#include <QtGlobal>
#include <QString>

struct ByteSelection
{
	qint64 begin, count;
	ByteSelection(qint64 begin, qint64 count)
		: begin(begin), count(count) {}
};

QString prettySize(qint64 bytes);

#endif // COMMON_H
