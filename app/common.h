#ifndef COMMON_H
#define COMMON_H

#include <QtGlobal>

struct ByteSelection
{
	qint64 begin, count;
	ByteSelection(qint64 begin, qint64 count)
		: begin(begin), count(count) {}
};

#endif // COMMON_H
