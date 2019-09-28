#include "mainwindow.h"
#include "hexview.h"

#include <QMessageBox>
#include <QMenuBar>
#include <QMenu>
#include <QAction>

#define DEFAULT_WINDOW_HEIGHT 480

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, m_hexView(new HexView)
	, m_file(QFile())
	, m_editor(nullptr)
{
	setCentralWidget(m_hexView);
	resize(m_hexView->width(), DEFAULT_WINDOW_HEIGHT);

	QMenu *fileMenu = new QMenu("&File");

	QAction *saveAction = new QAction("&Save");
	QAction *exitAction = new QAction("&Exit");

	saveAction->setShortcut(QKeySequence::Save);
	exitAction->setShortcut(QKeySequence::Quit);

	fileMenu->addAction(saveAction);
	fileMenu->addAction(exitAction);

	menuBar()->addMenu(fileMenu);

	connect(saveAction, &QAction::triggered, this, &MainWindow::saveChanges);
	connect(exitAction, &QAction::triggered, this, &MainWindow::onExitClicked);
}

bool MainWindow::openFile(const QString &path)
{
	m_file.setFileName(path);
	if (!m_file.open(QIODevice::ReadWrite)) {
		QMessageBox::critical(this, "",
							  QString("Failed to open file %1: %2").arg(path).arg(m_file.errorString()));
		return false;
	}
	m_editor = std::make_shared<BufferedEditor>(&m_file);
	m_hexView->setEditor(m_editor.get());
	resize(m_hexView->width(), DEFAULT_WINDOW_HEIGHT);

	return true;
}

bool MainWindow::saveChanges()
{
	if (!m_editor->writeChanges()) {
		QMessageBox::critical(this, "",
							  QString("Failed to save file %1: %2").
								arg(m_file.fileName()).
								arg(m_editor->errorString()));
		return false;
	}

	return true;
}

void MainWindow::onExitClicked()
{
	bool exit = true;

	if (m_editor->isModified()) {
		auto button = QMessageBox::question(this, "",
											QString("Save changes to %1?").arg(m_file.fileName()),
											QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
		if (button == QMessageBox::Yes) {
			if (!saveChanges())
				exit = false;
		} else if (button == QMessageBox::Cancel) {
			exit = false;
		}
	}

	if (exit)
		close();
}
