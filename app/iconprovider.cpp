#include "iconprovider.h"

#include <QWidget>

QIcon IconProvider::getContrastingIcon(const QString &name, QColor backgroundColor)
{
	if (backgroundColor.lightnessF() < 0.5)
		return getLightIcon(name);
	else
		return getDarkIcon(name);
}

QIcon IconProvider::getContrastingIcon(const QString &name, const QWidget *widget)
{
	return getContrastingIcon(name, widget->palette().window().color());
}
