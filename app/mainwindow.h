#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class BaseConverter;

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
	bool diffFiles(const QStringList &files);

	void onOpenClicked();
	bool saveChanges();
	bool closeTab(int index);
	void onExitClicked();
	void undo();
	void redo();
	void selectAll();
	void selectNone();
	void copyText();
	void copyHex();
	void openGotoDialog();
	void openFindDialog();
	void openBaseConverter();

private slots:
	void onTabCountChanged();
	void onCanUndoChanged();
	void onCanRedoChanged();
	void onSelectionChanged();

private:
	QTabWidget *m_tabWidget;

	QMenu *m_fileMenu;
	QMenu *m_editMenu;
	QMenu *m_toolsMenu;

	QAction *m_openAction;
	QAction *m_saveAction;
	QAction *m_exitAction;

	QAction *m_undoAction;
	QAction *m_redoAction;
	QAction *m_selectAllAction;
	QAction *m_selectNoneAction;
	QAction *m_copyTextAction;
	QAction *m_copyHexAction;
	QAction *m_gotoAction;
	QAction *m_findAction;

	QAction *m_baseConverterAction;

	BaseConverter *m_baseConverter;
};

#endif // MAINWINDOW_H
