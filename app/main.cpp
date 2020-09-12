#include "mainwindow.h"
#include <QApplication>
#include <QTextStream>

static QTextStream out(stdout);
static QTextStream err(stderr);

static void printUsage(const QStringList &args, QTextStream &out)
{
	out << "Usage: " << args[0] << " filename" << Qt::endl;
}

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	const auto &args = a.arguments();
	if (args.size() > 1 && (args[1] == "--help" || args[1] == "-h")) {
		printUsage(args, out);
		return 0;
	}

	MainWindow w;
	if (a.arguments()[1] == "--diff") {
		if (a.arguments().size() == 2) {
			printUsage(a.arguments(), err);
			return 1;
		}
		QStringList files;
		for (int i = 2; i < a.arguments().size(); ++i)
			files << a.arguments()[i];
		w.diffFiles(files);
	} else {
		for (int i = 1; i < a.arguments().size(); ++i)
			w.openFile(a.arguments()[i]);
	}
	w.show();

	return a.exec();
}
