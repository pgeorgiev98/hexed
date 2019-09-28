#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "bufferededitor.h"

#include <QMainWindow>
#include <QFile>

#include <memory>

class HexView;

class MainWindow : public QMainWindow
{
	Q_OBJECT
public:
	explicit MainWindow(QWidget *parent = nullptr);

public slots:
	bool openFile(const QString &path);

	bool saveChanges();
	void onExitClicked();

private:
	HexView *m_hexView;
	QFile m_file;
	std::shared_ptr<BufferedEditor> m_editor;
};

#endif // MAINWINDOW_H
