#include "mainwindow.h"
#include <QApplication>
#include <QTextStream>

static QTextStream out(stdout);
static QTextStream err(stderr);

static void printUsage(const QStringList &args, QTextStream &out)
{
	out << "Open files in tabs:" << Qt::endl
		<< "\t" << args[0] << " file1 [file2] [file3] [...]" << Qt::endl
		<< Qt::endl
		<< "Diff files:" << Qt::endl
		<< "\t" << args[0] << " --diff file1 [file2] [file3] [...]" << Qt::endl
		<< Qt::endl
		<< "Show this help message:" << Qt::endl
		<< "\t" << args[0] << " --help" << Qt::endl;
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
	if (args.size() > 1) {
		if (args[1] == "--diff") {
			if (args.size() == 2) {
				printUsage(args, err);
				return 1;
			}
			QStringList files;
			for (int i = 2; i < args.size(); ++i)
				files << args[i];
			w.diffFiles(files, true);
		} else {
			for (int i = 1; i < args.size(); ++i)
				w.openFile(args[i], true);
		}
	}
	w.show();

	return a.exec();
}
