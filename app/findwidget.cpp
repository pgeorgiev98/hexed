#include "findwidget.h"
#include <QHBoxLayout>
#include <QLabel>

FindWidget::FindWidget(QWidget *parent)
	: QWidget(parent)
{
	QHBoxLayout *layout = new QHBoxLayout;
	setLayout(layout);

	layout->addWidget(new QLabel("Need something?"));
}
