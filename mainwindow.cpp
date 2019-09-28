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
	, m_fileMenu(new QMenu("&File"))
	, m_openAction(new QAction("&Open"))
	, m_saveAction(new QAction("&Save"))
	, m_exitAction(new QAction("&Exit"))
{
	setCentralWidget(m_tabWidget);
	resize(640, 480);

	m_tabWidget->setTabsClosable(true);
	connect(m_tabWidget, &QTabWidget::tabCloseRequested, this, &MainWindow::closeTab);

	m_openAction->setShortcut(QKeySequence::Open);
	m_saveAction->setShortcut(QKeySequence::Save);
	m_exitAction->setShortcut(QKeySequence::Quit);

	m_fileMenu->addAction(m_openAction);
	m_fileMenu->addAction(m_saveAction);
	m_fileMenu->addAction(m_exitAction);

	menuBar()->addMenu(m_fileMenu);

	connect(m_openAction, &QAction::triggered, this, &MainWindow::onOpenClicked);
	connect(m_saveAction, &QAction::triggered, this, &MainWindow::saveChanges);
	connect(m_exitAction, &QAction::triggered, this, &MainWindow::onExitClicked);

	onTabCountChanged();
}

bool MainWindow::openFile(const QString &path)
{
	HexView *tab = new HexView;
	bool ok = tab->openFile(path);
	if (ok) {
		m_tabWidget->insertTab(m_tabWidget->count(), tab, path);
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


void MainWindow::onTabCountChanged()
{
	bool hasTabs = m_tabWidget->count() > 0;
	m_saveAction->setEnabled(hasTabs);
}
