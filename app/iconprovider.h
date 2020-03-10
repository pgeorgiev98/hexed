#ifndef ICONPROVIDER_H
#define ICONPROVIDER_H

#include <QIcon>
#include <QString>

class QWidget;

class IconProvider
{
public:
	IconProvider() = delete;

	static constexpr const char *upArrow = "up_arrow.svg";
	static constexpr const char *downArrow = "down_arrow.svg";
	static constexpr const char *cross = "cross.svg";

	static QIcon getLightIcon(const QString &name)
	{
		return QIcon(":/light/" + name);
	}

	static QIcon getDarkIcon(const QString &name)
	{
		return QIcon(":/dark/" + name);
	}

	static QIcon getContrastingIcon(const QString &name, QColor backgroundColor);

	static QIcon getContrastingIcon(const QString &name, const QWidget *widget);
};

#endif // ICONPROVIDER_H
