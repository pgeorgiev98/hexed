#include "mainwindow.h"
#include "findwidget.h"
#include "hexviewinternal.h"

#include <QMessageBox>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QTabWidget>
#include <QFileDialog>
#include <QStandardPaths>

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, m_tabWidget(new QTabWidget)
	, m_fileMenu(new QMenu("&File"))
	, m_editMenu(new QMenu("&Edit"))
	, m_openAction(new QAction("&Open"))
	, m_saveAction(new QAction("&Save"))
	, m_exitAction(new QAction("&Exit"))
	, m_undoAction(new QAction("&Undo"))
	, m_redoAction(new QAction("&Redo"))
	, m_gotoAction(new QAction("&Go to"))
{
	setCentralWidget(m_tabWidget);
	resize(640, 480);

	m_tabWidget->setTabsClosable(true);
	connect(m_tabWidget, &QTabWidget::tabCloseRequested, this, &MainWindow::closeTab);

	m_openAction->setShortcut(QKeySequence::Open);
	m_saveAction->setShortcut(QKeySequence::Save);
	m_exitAction->setShortcut(QKeySequence::Quit);
	m_undoAction->setShortcut(QKeySequence::Undo);
	m_redoAction->setShortcut(QKeySequence::Redo);
	m_gotoAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_G));

	m_fileMenu->addAction(m_openAction);
	m_fileMenu->addAction(m_saveAction);
	m_fileMenu->addAction(m_exitAction);

	m_editMenu->addAction(m_undoAction);
	m_editMenu->addAction(m_redoAction);
	m_editMenu->addSeparator();
	m_editMenu->addAction(m_gotoAction);

	menuBar()->addMenu(m_fileMenu);
	menuBar()->addMenu(m_editMenu);

	connect(m_openAction, &QAction::triggered, this, &MainWindow::onOpenClicked);
	connect(m_saveAction, &QAction::triggered, this, &MainWindow::saveChanges);
	connect(m_exitAction, &QAction::triggered, this, &MainWindow::onExitClicked);
	connect(m_undoAction, &QAction::triggered, this, &MainWindow::undo);
	connect(m_redoAction, &QAction::triggered, this, &MainWindow::redo);
	connect(m_gotoAction, &QAction::triggered, this, &MainWindow::openGotoDialog);

	onTabCountChanged();
}

bool MainWindow::openFile(const QString &path)
{
	HexViewInternal *tab = new HexViewInternal;
	bool ok = tab->openFile(path);
	if (ok) {
		m_tabWidget->insertTab(m_tabWidget->count(), tab, path);
		connect(tab, &HexViewInternal::canUndoChanged, this, &MainWindow::onCanUndoChanged);
		connect(tab, &HexViewInternal::canRedoChanged, this, &MainWindow::onCanRedoChanged);
		m_tabWidget->setCurrentWidget(tab);
		onTabCountChanged();
	}
	return ok;
}

void MainWindow::onOpenClicked()
{
	// TODO: Remember the last opened directory
	QStringList dirs = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
	QString dir;
	if (!dirs.isEmpty())
		dir = dirs.first();

	QString filename = QFileDialog::getOpenFileName(this, "Open file", dir);
	openFile(filename);
}

bool MainWindow::saveChanges()
{
	if (m_tabWidget->count() == 0)
		return true;
	HexViewInternal *tab = qobject_cast<HexViewInternal *>(m_tabWidget->currentWidget());
	Q_ASSERT(tab);
	return tab->saveChanges();
}

bool MainWindow::closeTab(int index)
{
	HexViewInternal *tab = qobject_cast<HexViewInternal *>(m_tabWidget->widget(index));
	Q_ASSERT(tab);
	if (!tab->quit())
		return false;

	m_tabWidget->removeTab(index);
	onTabCountChanged();
	return true;
}

void MainWindow::onExitClicked()
{
	for (int i = 0; i < m_tabWidget->count(); ++i)
		if (!closeTab(i))
			return;
	close();
}

void MainWindow::undo()
{
	HexViewInternal *tab = qobject_cast<HexViewInternal *>(m_tabWidget->currentWidget());
	Q_ASSERT(tab);
	tab->undo();
}

void MainWindow::redo()
{
	HexViewInternal *tab = qobject_cast<HexViewInternal *>(m_tabWidget->currentWidget());
	Q_ASSERT(tab);
	tab->redo();
}

void MainWindow::openGotoDialog()
{
	HexViewInternal *tab = qobject_cast<HexViewInternal *>(m_tabWidget->currentWidget());
	Q_ASSERT(tab);
	tab->openGotoDialog();
}


void MainWindow::onTabCountChanged()
{
	bool hasTabs = m_tabWidget->count() > 0;
	m_saveAction->setEnabled(hasTabs);
	m_gotoAction->setEnabled(hasTabs);
	onCanUndoChanged();
	onCanRedoChanged();
}

void MainWindow::onCanUndoChanged()
{
	if (m_tabWidget->count() == 0) {
		m_undoAction->setEnabled(false);
		return;
	}
	HexViewInternal *tab = qobject_cast<HexViewInternal *>(m_tabWidget->currentWidget());
	Q_ASSERT(tab);
	m_undoAction->setEnabled(tab->canUndo());
}

void MainWindow::onCanRedoChanged()
{
	if (m_tabWidget->count() == 0) {
		m_redoAction->setEnabled(false);
		return;
	}
	HexViewInternal *tab = qobject_cast<HexViewInternal *>(m_tabWidget->currentWidget());
	Q_ASSERT(tab);
	m_redoAction->setEnabled(tab->canRedo());
}
