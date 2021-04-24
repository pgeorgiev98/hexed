#include "difffilesdialog.h"
#include "utilities.h"
#include <QGridLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

DiffFilesDialog::DiffFilesDialog(QWidget *parent)
	: QDialog(parent)
{
	QLineEdit *file1 = new QLineEdit;
	QLineEdit *file2 = new QLineEdit;
	QPushButton *selectFile1 = new QPushButton("Browse");
	QPushButton *selectFile2 = new QPushButton("Browse");
	QPushButton *ok = new QPushButton("Compare");

	QGridLayout *layout = new QGridLayout;
	layout->addWidget(new QLabel("First file:"), 0, 0);
	layout->addWidget(file1, 0, 1);
	layout->addWidget(selectFile1, 0, 2);
	layout->addWidget(new QLabel("Second file:"), 1, 0);
	layout->addWidget(file2, 1, 1);
	layout->addWidget(selectFile2, 1, 2);
	layout->addWidget(ok, 2, 0, 1, 3, Qt::AlignHCenter);

	setLayout(layout);

	auto updateOk = [file1, file2, ok]() {
		ok->setEnabled(!file1->text().isEmpty() && !file2->text().isEmpty());
	};

	connect(selectFile1, &QPushButton::clicked, [this, file1]() {
		if (QString filename = selectFile(this); !filename.isEmpty())
			file1->setText(filename);
	});

	connect(selectFile2, &QPushButton::clicked, [this, file2]() {
		if (QString filename = selectFile(this); !filename.isEmpty())
			file2->setText(filename);
	});

	connect(file1, &QLineEdit::textChanged, updateOk);
	connect(file2, &QLineEdit::textChanged, updateOk);

	connect(ok, &QPushButton::clicked, [this, file1, file2]() {
		Q_ASSERT(!file1->text().isEmpty() && !file2->text().isEmpty());
		m_selectedFiles = QStringList{file1->text(), file2->text()};
		accept();
	});

	updateOk();

	auto width = 80 * file1->fontMetrics().averageCharWidth();
	file1->setMinimumWidth(width);
	file2->setMinimumWidth(width);
}

QStringList DiffFilesDialog::selectedFiles() const
{
	return m_selectedFiles;
}
