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
	void undo();
	void redo();
	void openGotoDialog();
	void openFindDialog();

private slots:
	void onTabCountChanged();
	void onCanUndoChanged();
	void onCanRedoChanged();

private:
	QTabWidget *m_tabWidget;

	QMenu *m_fileMenu;
	QMenu *m_editMenu;

	QAction *m_openAction;
	QAction *m_saveAction;
	QAction *m_exitAction;

	QAction *m_undoAction;
	QAction *m_redoAction;
	QAction *m_gotoAction;
	QAction *m_findAction;
};

#endif // MAINWINDOW_H
