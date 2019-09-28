#include "mainwindow.h"
#include <QApplication>
#include <QTextStream>

static void printUsage(const QStringList &args, QTextStream &out)
{
	out << "Usage: " << args[0] << " filename" << endl;
}

static QTextStream out(stdout);

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	const auto &args = a.arguments();
	if (args.size() == 1 || args[1] == "--help" || args[1] == "-h") {
		printUsage(args, out);
		return 0;
	}

	QString filename = argv[1];

	MainWindow w;
	if (!w.openFile(filename))
		return 1;
	w.show();

	return a.exec();
}
