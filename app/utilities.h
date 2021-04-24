#ifndef UTILITIES_H
#define UTILITIES_H

#include <QString>
#include <QFileDialog>
#include <QStandardPaths>

static QString selectFile(QWidget *parent)
{
	// TODO: Remember the last opened directory
	QStringList dirs = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
	QString dir;
	if (!dirs.isEmpty())
		dir = dirs.first();

	return QFileDialog::getOpenFileName(parent, "Select file", dir);
}

#endif // UTILITIES_H
