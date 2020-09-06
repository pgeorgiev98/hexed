#ifndef HEXVIEW_H
#define HEXVIEW_H

#include "common.h"

#include <QWidget>
#include <optional>

class HexViewInternal;
class QScrollBar;
class QStatusBar;
class QLabel;

class HexView : public QWidget
{
	Q_OBJECT
public:
	explicit HexView(QWidget *parent = nullptr);

	std::optional<ByteSelection> selection() const;

public slots:
	bool canUndo() const;
	bool canRedo() const;

	bool openFile(const QString &path);
	bool saveChanges();
	bool quit();
	void undo();
	void redo();
	void selectAll();
	void selectNone();
	void copyText();
	void copyHex();
	void openGotoDialog();
	void openFindDialog();

signals:
	void canUndoChanged(bool canUndo);
	void canRedoChanged(bool canRedo);
	void selectionChanged();

private slots:
	void updateScrollMaximum();
	void setTopRow(qint64 topRow);
	void onScrollBarChanged(int value);
	void updateStatusBar();

	int scrollStep(qint64 rowCount) const;

private:
	HexViewInternal *m_hexViewInternal;
	QScrollBar *m_verticalScrollBar;
	QStatusBar *m_statusBar;
	QLabel *m_fileSizeLabel;
	QLabel *m_selectionLabel;
};

#endif // HEXVIEW_H
