#ifndef HEXVIEW_H
#define HEXVIEW_H

#include "common.h"

#include <QWidget>
#include <QVector>

#include <optional>

class HexViewInternal;
class BufferedEditor;

class QHBoxLayout;
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
	BufferedEditor *editor();

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
	void updateViews();
	void setTopRow(qint64 topRow);
	void onScrollBarChanged(int value);
	void updateStatusBar();
	void onUserChangedSelection();
	void onViewFocusedSlot();
	void onViewFocused();

	int scrollStep(qint64 rowCount) const;
	qint64 scrollMaximum() const;
	HexViewInternal *hexViewInternal();
	const HexViewInternal *hexViewInternal() const;

private:
	QVector<HexViewInternal *> m_hexViews;
	QHBoxLayout *m_hexViewsLayout;
	QScrollBar *m_verticalScrollBar;
	QStatusBar *m_statusBar;
	QLabel *m_fileSizeLabel;
	QLabel *m_selectionLabel;

	int m_focusedViewIndex;
};

#endif // HEXVIEW_H
