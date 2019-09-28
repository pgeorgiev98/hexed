#include "mainwindow.h"
#include "hexview.h"

#include <QMessageBox>

#define DEFAULT_WINDOW_HEIGHT 480

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, m_hexView(new HexView)
	, m_file(QFile())
	, m_editor(nullptr)
{
	setCentralWidget(m_hexView);
	resize(m_hexView->width(), DEFAULT_WINDOW_HEIGHT);
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
