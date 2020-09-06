#ifndef COMMON_H
#define COMMON_H

#include <QtGlobal>
#include <QString>

struct ByteSelection
{
	enum Type { Cells, Text };
	ByteSelection(qint64 begin, qint64 count, Type type = Type::Cells)
		: begin(begin), count(count), type(type) {}

	qint64 begin, count;
	Type type;

};

QString prettySize(qint64 bytes);

#endif // COMMON_H
