#include "findwidget.h"

#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

#include <QKeyEvent>

FindWidget::FindWidget(QWidget *parent)
	: QWidget(parent)
	, m_input(new QLineEdit)
{
	setAutoFillBackground(true);
	m_input->setPlaceholderText("0D FF E2 ...");

	QPushButton *up = new QPushButton;
	QPushButton *down = new QPushButton;
	QPushButton *close = new QPushButton;
	// TODO: Choose icon based on background color
	up->setIcon(QIcon(":/light/up_arrow.svg"));
	up->setDisabled(true);
	down->setIcon(QIcon(":/light/down_arrow.svg"));
	down->setDisabled(true);
	close->setIcon(QIcon(":/light/x.svg"));

	QHBoxLayout *layout = new QHBoxLayout;
	setLayout(layout);

	layout->addWidget(m_input);
	layout->addWidget(up);
	layout->addWidget(down);
	layout->addWidget(close);

	connect(close, &QPushButton::clicked, this, &FindWidget::close);
	connect(m_input, &QLineEdit::textChanged, [up, down](const QString &s) {
		up->setDisabled(s.isEmpty());
		down->setDisabled(s.isEmpty());
	});
}

void FindWidget::close()
{
	hide();
	emit closed();
}

void FindWidget::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Escape) {
		close();
	} else {
		QWidget::keyPressEvent(event);
	}
}

void FindWidget::focusInEvent(QFocusEvent *)
{
	m_input->setFocus();
	m_input->selectAll();
}
