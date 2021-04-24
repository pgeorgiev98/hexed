#ifndef DIFFFILESDIALOG_H
#define DIFFFILESDIALOG_H

#include <QDialog>
#include <QStringList>

class DiffFilesDialog : public QDialog
{
	Q_OBJECT
public:
	explicit DiffFilesDialog(QWidget *parent = nullptr);
	QStringList selectedFiles() const;

private:
	QStringList m_selectedFiles;
};

#endif // DIFFFILESDIALOG_H
