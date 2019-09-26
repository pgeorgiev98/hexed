#include "hexview.h"
#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	HexView w;
	w.show();

	return a.exec();
}
