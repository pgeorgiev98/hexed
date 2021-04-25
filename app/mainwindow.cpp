#include "mainwindow.h"
#include "utilities.h"
#include "hexview.h"
#include "baseconverter.h"
#include "difffilesdialog.h"
#include "bufferededitor.h"

#include <QMessageBox>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QTabWidget>
#include <QScreen>

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, m_tabWidget(new QTabWidget)
	, m_fileMenu(new QMenu("&File"))
	, m_editMenu(new QMenu("&Edit"))
	, m_toolsMenu(new QMenu("&Tools"))
	, m_openAction(new QAction("&Open"))
	, m_diffFiles(new QAction("Compare files"))
	, m_diffCurrentFile(new QAction("Compare current file"))
	, m_saveAction(new QAction("&Save"))
	, m_exitAction(new QAction("&Exit"))
	, m_undoAction(new QAction("&Undo"))
	, m_redoAction(new QAction("&Redo"))
	, m_selectAllAction(new QAction("Select &All"))
	, m_selectNoneAction(new QAction("Select &None"))
	, m_copyTextAction(new QAction("Copy &text"))
	, m_copyHexAction(new QAction("Copy &hex"))
	, m_gotoAction(new QAction("&Go to"))
	, m_findAction(new QAction("&Find"))
	, m_baseConverterAction(new QAction("Base &Converter"))
	, m_baseConverter(new BaseConverter(this))
{
	setCentralWidget(m_tabWidget);
	resize(640, 480);

	m_baseConverter->hide();

	m_tabWidget->setTabsClosable(true);
	connect(m_tabWidget, &QTabWidget::tabCloseRequested, this, &MainWindow::closeTab);

	m_openAction->setShortcut(QKeySequence::Open);
	m_saveAction->setShortcut(QKeySequence::Save);
	m_exitAction->setShortcut(QKeySequence::Quit);
	m_undoAction->setShortcut(QKeySequence::Undo);
	m_redoAction->setShortcut(QKeySequence::Redo);
	m_gotoAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_G));
	m_findAction->setShortcut(QKeySequence::Find);

	m_fileMenu->addAction(m_openAction);
	m_fileMenu->addAction(m_diffFiles);
	m_fileMenu->addAction(m_diffCurrentFile);
	m_fileMenu->addAction(m_saveAction);
	m_fileMenu->addAction(m_exitAction);

	m_editMenu->addAction(m_undoAction);
	m_editMenu->addAction(m_redoAction);
	m_editMenu->addSeparator();
	m_editMenu->addAction(m_selectAllAction);
	m_editMenu->addAction(m_selectNoneAction);
	m_editMenu->addSeparator();
	m_editMenu->addAction(m_copyTextAction);
	m_editMenu->addAction(m_copyHexAction);
	m_editMenu->addSeparator();
	m_editMenu->addAction(m_gotoAction);
	m_editMenu->addAction(m_findAction);

	m_toolsMenu->addAction(m_baseConverterAction);

	menuBar()->addMenu(m_fileMenu);
	menuBar()->addMenu(m_editMenu);
	menuBar()->addMenu(m_toolsMenu);

	connect(m_openAction, &QAction::triggered, this, &MainWindow::onOpenClicked);
	connect(m_diffFiles, &QAction::triggered, this, qOverload<>(&MainWindow::diffFiles));
	connect(m_diffCurrentFile, &QAction::triggered, this, &MainWindow::diffCurrentFile);
	connect(m_saveAction, &QAction::triggered, this, &MainWindow::saveChanges);
	connect(m_exitAction, &QAction::triggered, this, &MainWindow::onExitClicked);

	connect(m_undoAction, &QAction::triggered, this, &MainWindow::undo);
	connect(m_redoAction, &QAction::triggered, this, &MainWindow::redo);
	connect(m_selectAllAction, &QAction::triggered, this, &MainWindow::selectAll);
	connect(m_selectNoneAction, &QAction::triggered, this, &MainWindow::selectNone);
	connect(m_copyTextAction, &QAction::triggered, this, &MainWindow::copyText);
	connect(m_copyHexAction, &QAction::triggered, this, &MainWindow::copyHex);
	connect(m_gotoAction, &QAction::triggered, this, &MainWindow::openGotoDialog);
	connect(m_findAction, &QAction::triggered, this, &MainWindow::openFindDialog);

	connect(m_baseConverterAction, &QAction::triggered, this, &MainWindow::openBaseConverter);

	onTabCountChanged();
}

bool MainWindow::openFile(const QString &path, bool resizeWidth)
{
	HexView *tab = new HexView;
	bool ok = tab->openFile(path);
	if (ok) {
		m_tabWidget->insertTab(m_tabWidget->count(), tab, path);
		connect(tab, &HexView::canUndoChanged, this, &MainWindow::onCanUndoChanged);
		connect(tab, &HexView::canRedoChanged, this, &MainWindow::onCanRedoChanged);
		connect(tab, &HexView::selectionChanged, this, &MainWindow::onSelectionChanged);
		m_tabWidget->setCurrentWidget(tab);
		onTabCountChanged();
		if (resizeWidth)
			updateWindowWidth();
	}
	return ok;
}

bool MainWindow::diffFiles(const QStringList &files, bool resizeWidth)
{
	HexView *tab = new HexView;

	bool ok = true;
	for (const QString &file : files)
		ok &= tab->openFile(file);

	if (ok) {
		m_tabWidget->insertTab(m_tabWidget->count(), tab, files.join(" | "));
		connect(tab, &HexView::canUndoChanged, this, &MainWindow::onCanUndoChanged);
		connect(tab, &HexView::canRedoChanged, this, &MainWindow::onCanRedoChanged);
		connect(tab, &HexView::selectionChanged, this, &MainWindow::onSelectionChanged);
		m_tabWidget->setCurrentWidget(tab);
		onTabCountChanged();
		if (resizeWidth)
			updateWindowWidth();
	} else {
		// TODO: proper cleanup
	}

	return ok;
}

void MainWindow::onOpenClicked()
{
	QString filename = selectFile(this);
	if (!filename.isEmpty())
		openFile(filename);
}

void MainWindow::diffFiles()
{
	DiffFilesDialog dialog(this);

	if (!dialog.exec())
		return;

	diffFiles(dialog.selectedFiles());
}

void MainWindow::diffCurrentFile()
{
	Q_ASSERT(m_tabWidget->count() > 0);

	QString filename = selectFile(this);
	if (filename.isEmpty())
		return;

	HexView *tab = qobject_cast<HexView *>(m_tabWidget->currentWidget());
	Q_ASSERT(tab);
	tab->openFile(filename);
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

void MainWindow::undo()
{
	HexView *tab = qobject_cast<HexView *>(m_tabWidget->currentWidget());
	Q_ASSERT(tab);
	tab->undo();
}

void MainWindow::redo()
{
	HexView *tab = qobject_cast<HexView *>(m_tabWidget->currentWidget());
	Q_ASSERT(tab);
	tab->redo();
}

void MainWindow::selectAll()
{
	HexView *tab = qobject_cast<HexView *>(m_tabWidget->currentWidget());
	Q_ASSERT(tab);
	tab->selectAll();
}

void MainWindow::selectNone()
{
	HexView *tab = qobject_cast<HexView *>(m_tabWidget->currentWidget());
	Q_ASSERT(tab);
	tab->selectNone();
}

void MainWindow::copyText()
{
	HexView *tab = qobject_cast<HexView *>(m_tabWidget->currentWidget());
	Q_ASSERT(tab);
	tab->copyText();
}

void MainWindow::copyHex()
{
	HexView *tab = qobject_cast<HexView *>(m_tabWidget->currentWidget());
	Q_ASSERT(tab);
	tab->copyHex();
}

void MainWindow::openGotoDialog()
{
	HexView *tab = qobject_cast<HexView *>(m_tabWidget->currentWidget());
	Q_ASSERT(tab);
	tab->openGotoDialog();
}

void MainWindow::openFindDialog()
{
	HexView *tab = qobject_cast<HexView *>(m_tabWidget->currentWidget());
	Q_ASSERT(tab);
	tab->openFindDialog();
}

void MainWindow::openBaseConverter()
{
	m_baseConverter->show();
	m_baseConverter->activateWindow();
	m_baseConverter->raise();
}

void MainWindow::updateWindowWidth()
{
	int screenWidth = window()->screen()->size().width();
	int optimalWidth = 0;
	for (int i = 0; i < m_tabWidget->count(); ++i) {
		HexView *tab = static_cast<HexView *>(m_tabWidget->widget(i));
		optimalWidth = qMax(optimalWidth, tab->optimalWidth() + 10);
	}
	int newWidth = qMin(screenWidth, optimalWidth);
	resize(newWidth, height());
}


void MainWindow::onTabCountChanged()
{
	bool hasTabs = m_tabWidget->count() > 0;
	m_diffCurrentFile->setEnabled(hasTabs);
	m_saveAction->setEnabled(hasTabs);
	m_gotoAction->setEnabled(hasTabs);
	m_findAction->setEnabled(hasTabs);
	m_selectAllAction->setEnabled(hasTabs);
	onCanUndoChanged();
	onCanRedoChanged();
	onSelectionChanged();
}

void MainWindow::onCanUndoChanged()
{
	if (m_tabWidget->count() == 0) {
		m_undoAction->setEnabled(false);
		return;
	}
	HexView *tab = qobject_cast<HexView *>(m_tabWidget->currentWidget());
	Q_ASSERT(tab);
	m_undoAction->setEnabled(tab->canUndo());
}

void MainWindow::onCanRedoChanged()
{
	if (m_tabWidget->count() == 0) {
		m_redoAction->setEnabled(false);
		return;
	}
	HexView *tab = qobject_cast<HexView *>(m_tabWidget->currentWidget());
	Q_ASSERT(tab);
	m_redoAction->setEnabled(tab->canRedo());
}

void MainWindow::onSelectionChanged()
{
	bool hasSelection = false;
	if (m_tabWidget->count() > 0) {
		HexView *tab = qobject_cast<HexView *>(m_tabWidget->currentWidget());
		Q_ASSERT(tab);
		hasSelection = tab->selection().has_value();

		auto selection = tab->selection();
		auto editor = tab->editor();
		if (selection && selection->count <= 8 && selection->begin != editor->size()) {
			editor->seek(selection->begin);
			QByteArray bytes;
			for (int i = 0; i < selection->count; ++i)
				bytes.append(quint8(*(editor->getByte().current)));
			m_baseConverter->setFromBytes(bytes);
		}
	}
	m_selectNoneAction->setEnabled(hasSelection);
	m_copyTextAction->setEnabled(hasSelection);
	m_copyHexAction->setEnabled(hasSelection);
}
