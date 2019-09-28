#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QTabWidget;
class QMenu;
class QAction;

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

private slots:
	void onTabCountChanged();

private:
	QTabWidget *m_tabWidget;

	QMenu *m_fileMenu;
	QAction *m_openAction;
	QAction *m_saveAction;
	QAction *m_exitAction;
};

#endif // MAINWINDOW_H
