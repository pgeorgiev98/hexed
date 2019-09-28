#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QTabWidget;

class MainWindow : public QMainWindow
{
	Q_OBJECT
public:
	explicit MainWindow(QWidget *parent = nullptr);

public slots:
	bool openFile(const QString &path);

	void onOpenClicked();
	bool saveChanges();
	bool closeTab(int index);
	void onExitClicked();

private:
	QTabWidget *m_tabWidget;
};

#endif // MAINWINDOW_H
