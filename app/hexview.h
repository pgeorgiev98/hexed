#ifndef HEXVIEW_H
#define HEXVIEW_H

#include <QWidget>

class HexViewInternal;
class QScrollBar;

class HexView : public QWidget
{
	Q_OBJECT
public:
	explicit HexView(QWidget *parent = nullptr);

public slots:
	bool canUndo() const;
	bool canRedo() const;

	bool openFile(const QString &path);
	bool saveChanges();
	bool quit();
	void undo();
	void redo();
	void openGotoDialog();

signals:
	void canUndoChanged(bool canUndo);
	void canRedoChanged(bool canRedo);

private slots:
	void onRowCountChanged();
	void onTopRowChanged(int topRow);
	void onScrollBarChanged(int value);

private:
	HexViewInternal *m_hexViewInternal;
	QScrollBar *m_verticalScrollBar;
};

#endif // HEXVIEW_H
