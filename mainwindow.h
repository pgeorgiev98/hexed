#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "bufferededitor.h"

#include <QMainWindow>
#include <QFile>

class HexView;

class MainWindow : public QMainWindow
{
	Q_OBJECT
public:
	explicit MainWindow(QWidget *parent = nullptr);

public slots:
	bool openFile(const QString &path);

private:
	HexView *m_hexView;
	QFile m_file;
	BufferedEditor m_editor;
};

#endif // MAINWINDOW_H
