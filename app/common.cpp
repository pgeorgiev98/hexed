#include "common.h"

QString prettySize(qint64 bytes)
{
	static const char *suffix[] = {"B", "KiB", "MiB", "GiB", "TiB", "PiB"};
	int suffixIndex = 0;
	int remainder = 0;
	while (bytes >= 1024) {
		remainder = bytes % 1024;
		bytes /= 1024;
		++suffixIndex;
	}
	if (suffixIndex == 0)
		return QString("%1%2").arg(bytes).arg(suffix[suffixIndex]);
	else
		return QString("%1.%2%3").arg(bytes).arg(int(10.0 * remainder / 1024.0)).arg(suffix[suffixIndex]);
}
