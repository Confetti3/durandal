#include "EditorWidget.h"
#include "MarkdownHighlighter.h"
#include "AutoCompleter.h"

#include <QPainter>
#include <QTextBlock>
#include <QScrollBar>
#include <QTextDocumentFragment>
#include <QFileInfo>
#include <QTextStream>
#include <QRegularExpression>

EditorWidget::EditorWidget(QWidget* parent)
    : QPlainTextEdit(parent)
    , m_highlighter(new MarkdownHighlighter(document()))
    , m_completer(new AutoCompleter(this, this))
    , m_lineNumberArea(new LineNumberArea(this))
    , m_wordCount(0)
{
    setLineWrapMode(QPlainTextEdit::WidgetWidth);
    setTabStopDistance(40);

    QFont font("Consolas", 13);
    font.setStyleHint(QFont::Monospace);
    document()->setDefaultFont(font);

    connect(this, &QPlainTextEdit::blockCountChanged,
            this, &EditorWidget::updateLineNumberAreaWidth);
    connect(this, &QPlainTextEdit::updateRequest,
            this, &EditorWidget::updateLineNumberArea);
    connect(this, &QPlainTextEdit::cursorPositionChanged,
            this, &EditorWidget::highlightCurrentLine);
    connect(this, &QPlainTextEdit::cursorPositionChanged,
            this, &EditorWidget::emitCursorPosition);

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
}

void EditorWidget::setFileName(const QString& fileName)
{
    m_fileName = fileName;
    emit fileNameChanged(fileName);
}

QString EditorWidget::fileName() const { return m_fileName; }

void EditorWidget::setWikilinks(const QStringList& links)
{
    m_completer->setWikilinks(links);
}

void EditorWidget::setTags(const QStringList& tags)
{
    m_completer->setTags(tags);
}

QString EditorWidget::documentTitle() const
{
    QString text = toPlainText();
    QTextStream stream(&text);
    QString firstLine = stream.readLine().trimmed();
    if (firstLine.startsWith("# ")) {
        return firstLine.mid(2).trimmed();
    }
    return QFileInfo(m_fileName).completeBaseName();
}

int EditorWidget::wordCount() const
{
    QString text = toPlainText().trimmed();
    if (text.isEmpty()) return 0;
    return text.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts).size();
}

void EditorWidget::insertBold()
{
    QTextCursor cursor = textCursor();
    if (cursor.hasSelection()) {
        QString text = cursor.selectedText();
        cursor.insertText(QStringLiteral("**%1**").arg(text));
    } else {
        cursor.insertText("****");
        cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, 2);
        setTextCursor(cursor);
    }
}

void EditorWidget::insertItalic()
{
    QTextCursor cursor = textCursor();
    if (cursor.hasSelection()) {
        QString text = cursor.selectedText();
        cursor.insertText(QStringLiteral("*%1*").arg(text));
    } else {
        cursor.insertText("**");
        cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, 1);
        setTextCursor(cursor);
    }
}

void EditorWidget::insertHeading(int level)
{
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::StartOfLine);
    QString prefix = QString(level, '#') + " ";
    cursor.insertText(prefix);
}

void EditorWidget::insertBulletList()
{
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::StartOfLine);
    cursor.insertText("- ");
}

void EditorWidget::insertNumberedList()
{
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::StartOfLine);
    cursor.insertText("1. ");
}

void EditorWidget::insertCodeBlock()
{
    QTextCursor cursor = textCursor();
    cursor.insertText("\n```\n\n```\n");
    cursor.movePosition(QTextCursor::Up, QTextCursor::MoveAnchor, 2);
    setTextCursor(cursor);
}

void EditorWidget::insertLink()
{
    QTextCursor cursor = textCursor();
    if (cursor.hasSelection()) {
        QString text = cursor.selectedText();
        cursor.insertText(QStringLiteral("[%1](url)").arg(text));
    } else {
        cursor.insertText("[text](url)");
        cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, 5);
        cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, 4);
        setTextCursor(cursor);
    }
}

void EditorWidget::insertWikilink()
{
    QTextCursor cursor = textCursor();
    if (cursor.hasSelection()) {
        QString text = cursor.selectedText();
        cursor.insertText(QStringLiteral("[[%1]]").arg(text));
    } else {
        cursor.insertText("[[]]");
        cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, 2);
        setTextCursor(cursor);
    }
}

void EditorWidget::resizeEvent(QResizeEvent* event)
{
    QPlainTextEdit::resizeEvent(event);

    QRect cr = contentsRect();
    m_lineNumberArea->setGeometry(
        QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void EditorWidget::updateLineNumberAreaWidth(int)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void EditorWidget::updateLineNumberArea(const QRect& rect, int dy)
{
    if (dy)
        m_lineNumberArea->scroll(0, dy);
    else
        m_lineNumberArea->update(0, rect.y(), m_lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void EditorWidget::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;
        QColor base = palette().color(QPalette::Base);
        bool isDark = base.lightness() < 128;
        QColor lineColor = isDark ? base.lighter(108) : QColor("#e8f0fe");
        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
}

void EditorWidget::emitCursorPosition()
{
    QTextCursor cursor = textCursor();
    int line = cursor.blockNumber() + 1;
    int col = cursor.columnNumber() + 1;
    emit cursorPositionChanged(line, col);
}

int EditorWidget::lineNumberAreaWidth() const
{
    int digits = 1;
    int max = std::max(1, blockCount());
    while (max >= 10) { max /= 10; ++digits; }

    int space = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
    return space;
}

void EditorWidget::lineNumberAreaPaintEvent(QPaintEvent* event)
{
    QPainter painter(m_lineNumberArea);
    QColor base = palette().color(QPalette::Base);
    bool isDark = base.lightness() < 128;
    QColor bg = isDark ? base.darker(106) : QColor("#f0f0f0");
    QColor textColor = isDark ? QColor("#6e7681") : QColor("#888888");

    painter.fillRect(event->rect(), bg);

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(textColor);
            painter.drawText(0, top, m_lineNumberArea->width() - 3,
                             fontMetrics().height(),
                             Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}
