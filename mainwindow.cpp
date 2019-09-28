#include "mainwindow.h"
#include "hexview.h"

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
{
	setCentralWidget(m_tabWidget);
	resize(640, 480);

	m_tabWidget->setTabsClosable(true);
	connect(m_tabWidget, &QTabWidget::tabCloseRequested, this, &MainWindow::closeTab);

	QMenu *fileMenu = new QMenu("&File");

	QAction *openAction = new QAction("&Open");
	QAction *saveAction = new QAction("&Save");
	QAction *exitAction = new QAction("&Exit");

	openAction->setShortcut(QKeySequence::Open);
	saveAction->setShortcut(QKeySequence::Save);
	exitAction->setShortcut(QKeySequence::Quit);

	fileMenu->addAction(openAction);
	fileMenu->addAction(saveAction);
	fileMenu->addAction(exitAction);

	menuBar()->addMenu(fileMenu);

	connect(openAction, &QAction::triggered, this, &MainWindow::onOpenClicked);
	connect(saveAction, &QAction::triggered, this, &MainWindow::saveChanges);
	connect(exitAction, &QAction::triggered, this, &MainWindow::onExitClicked);
}

bool MainWindow::openFile(const QString &path)
{
	HexView *tab = new HexView;
	bool ok = tab->openFile(path);
	if (ok)
		m_tabWidget->insertTab(m_tabWidget->count(), tab, path);
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
	HexView *tab = qobject_cast<HexView *>(m_tabWidget->currentWidget());
	Q_ASSERT(tab);
	return tab->saveChanges();
}

bool MainWindow::closeTab(int index)
{
	HexView *tab = qobject_cast<HexView *>(m_tabWidget->widget(index));
	Q_ASSERT(tab);
	if (!tab->quit())
		return false;

	m_tabWidget->removeTab(index);
	return true;
}

void MainWindow::onExitClicked()
{
	for (int i = 0; i < m_tabWidget->count(); ++i)
		if (!closeTab(i))
			return;
	close();
}
