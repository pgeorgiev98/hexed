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
	if (args.size() > 1 && (args[1] == "--help" || args[1] == "-h")) {
		printUsage(args, out);
		return 0;
	}

	MainWindow w;
	for (int i = 1; i < a.arguments().size(); ++i)
		w.openFile(a.arguments()[i]);
	w.show();

	return a.exec();
}
