#ifndef HEXVIEWINTERNAL_H
#define HEXVIEWINTERNAL_H

#include "common.h"
#include "bufferededitor.h"

#include <QWidget>
#include <QString>
#include <QByteArray>
#include <QFont>
#include <QFontMetrics>
#include <QMap>
#include <QFile>

#include <optional>

class GotoDialog;
class FindWidget;

class QScrollBar;

class HexViewInternal : public QWidget
{
	Q_OBJECT
private:
	friend class HexView;
	friend class FindWidget;

	enum class Mode {
		Normal, Diff,
	};

	explicit HexViewInternal(QWidget *parent = nullptr);

	BufferedEditor *editor();
	QString toPlainText();
	QPoint getByteCoordinates(qint64 index) const;
	std::optional<ByteSelection> selection() const;
	qint64 rowCount() const;
	qint64 scrollMaximum() const;
	int bytesPerLine() const;
	bool canUndo() const;
	bool canRedo() const;
	bool cursorIsInFindWidget(QPoint cursorPos) const;

	void setMode(Mode mode);
	void setDiffGroup(QVector<HexViewInternal *> hexViews);

signals:
	void canUndoChanged(bool canUndo);
	void canRedoChanged(bool canRedo);
	void topRowChanged(qint64 topRow);
	void rowCountChanged();
	void scrollMaximumChanged();
	void userChangedSelection();
	void selectionChanged();

private slots:
	void setBytesPerLine(int bytesPerLine);
	void highlight(ByteSelection selection);
	void selectAll();
	void selectNone();
	void copy(ByteSelection selection);
	void setFont(QFont font);
	void setTopRow(qint64 topRow);
	bool openFile(const QString &path);
	bool saveChanges();
	bool quit();
	void undo();
	void redo();
	void openGotoDialog();
	void openFindDialog();
	void updateFindDialogPosition();

private:
	void setSelection(ByteSelection selection);

protected:
	void paintEvent(QPaintEvent *) override;
	void resizeEvent(QResizeEvent *) override;
	void mouseMoveEvent(QMouseEvent *) override;
	void mousePressEvent(QMouseEvent *) override;
	void mouseReleaseEvent(QMouseEvent *) override;
	void mouseDoubleClickEvent(QMouseEvent *) override;
	void leaveEvent(QEvent *) override;
	void wheelEvent(QWheelEvent *) override;
	void keyPressEvent(QKeyEvent *) override;

private:
	Mode m_mode;

	QFont m_font;
	QFontMetrics m_fontMetrics;
	int m_characterWidth;
	int m_cellSize, m_cellPadding;
	int m_bytesPerLine;
	qint64 m_hoveredIndex;

	std::optional<ByteSelection> m_selection;
	bool m_selectingRows;
	bool m_selecting;

	QFile m_file;
	BufferedEditor *m_editor;
	qint64 m_topRow;
	double m_mouseScrollBuffer;
	bool m_editingCell;
	char m_editingCellByte;

	GotoDialog *m_gotoDialog;
	FindWidget *m_findWidget;

	QVector<BufferedEditor::Byte> m_visiblePage;
	QVector<HexViewInternal *> m_diffGroup;

	qint64 getHoverCell(const QPoint &mousePos) const;
	qint64 getHoverText(const QPoint &mousePos) const;
	int lineNumberDigitsCount() const;
	int lineNumberWidth() const;
	void updateVisiblePage();
};

#endif // HEXVIEWINTERNAL_H
