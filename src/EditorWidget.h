#ifndef EDITORWIDGET_H
#define EDITORWIDGET_H

#include <QPlainTextEdit>
#include <QWidget>
#include <QCompleter>

class MarkdownHighlighter;
class AutoCompleter;

class EditorWidget : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit EditorWidget(QWidget* parent = nullptr);

    void setFileName(const QString& fileName);
    QString fileName() const;

    void setWikilinks(const QStringList& links);
    void setTags(const QStringList& tags);
    QString documentTitle() const;

    int wordCount() const;

    void insertBold();
    void insertItalic();
    void insertHeading(int level);
    void insertBulletList();
    void insertNumberedList();
    void insertCodeBlock();
    void insertLink();
    void insertWikilink();

    int fontSize() const;
    void setFontSize(int size);

signals:
    void fileNameChanged(const QString& fileName);
    void cursorPositionChanged(int line, int column);
    void fontSizeChanged(int size);

protected:
    void resizeEvent(QResizeEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;

public slots:
    void highlightCurrentLine();

private slots:
    void updateLineNumberAreaWidth(int blockCount);
    void updateLineNumberArea(const QRect& rect, int dy);
    void emitCursorPosition();

private:
    QString m_fileName;
    MarkdownHighlighter* m_highlighter;
    AutoCompleter* m_completer;
    QWidget* m_lineNumberArea;
    int m_wordCount;
    int m_fontSize;

    int lineNumberAreaWidth() const;
    void lineNumberAreaPaintEvent(QPaintEvent* event);
    void updateFont();

    class LineNumberArea : public QWidget
    {
    public:
        explicit LineNumberArea(EditorWidget* editor)
            : QWidget(editor), m_editor(editor) {}
        QSize sizeHint() const override {
            return QSize(m_editor->lineNumberAreaWidth(), 0);
        }
    protected:
        void paintEvent(QPaintEvent* event) override {
            m_editor->lineNumberAreaPaintEvent(event);
        }
    private:
        EditorWidget* m_editor;
    };
};

#endif // EDITORWIDGET_H
