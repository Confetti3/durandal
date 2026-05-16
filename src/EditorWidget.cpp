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
#include <QMenu>
#include <QAction>
#include <QFont>

EditorWidget::EditorWidget(QWidget* parent)
    : QPlainTextEdit(parent)
    , m_highlighter(new MarkdownHighlighter(document()))
    , m_completer(new AutoCompleter(this, this))
    , m_lineNumberArea(new LineNumberArea(this))
    , m_wordCount(0)
    , m_fontSize(13)
{
    setLineWrapMode(QPlainTextEdit::WidgetWidth);
    setTabStopDistance(40);

    QFont font("Consolas", 13);
    font.setStyleHint(QFont::Monospace);
    document()->setDefaultFont(font);
    setFont(font);
    m_lineNumberArea->setFont(font);

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

int EditorWidget::fontSize() const
{
    return m_fontSize;
}

void EditorWidget::setWordWrap(bool wrap)
{
    setLineWrapMode(wrap ? WidgetWidth : NoWrap);
}

bool EditorWidget::wordWrap() const
{
    return lineWrapMode() == WidgetWidth;
}

void EditorWidget::setFontFamily(const QString& family)
{
    QFont font(family, m_fontSize);
    font.setStyleHint(QFont::Monospace);
    document()->setDefaultFont(font);
    setFont(font);
    m_lineNumberArea->setFont(font);
}

void EditorWidget::setTabWidth(int spaces)
{
    QFontMetrics fm(font());
    int width = fm.horizontalAdvance(QString(spaces, ' '));
    setTabStopDistance(width);
}

void EditorWidget::setShowLineNumbers(bool show)
{
    m_lineNumberArea->setVisible(show);
    updateLineNumberAreaWidth(0);
}

void EditorWidget::setFontSize(int size)
{
    if (size < 6 || size > 48) return;
    m_fontSize = size;

    QTextCursor cursor = textCursor();

    if (cursor.hasSelection()) {
        double emValue = size / 14.0;
        QString openTag = QStringLiteral("<span style=\"font-size:%1em\">")
                              .arg(emValue, 0, 'f', 2);
        QString closeTag = QStringLiteral("</span>");

        int start = cursor.selectionStart();
        int end = cursor.selectionEnd();

        cursor.beginEditBlock();
        cursor.setPosition(end);
        cursor.insertText(closeTag);
        cursor.setPosition(start);
        cursor.insertText(openTag);
        // Reselect the original text between the tags
        cursor.setPosition(start + openTag.length());
        cursor.setPosition(end + openTag.length(), QTextCursor::KeepAnchor);
        cursor.endEditBlock();
        setTextCursor(cursor);
    }

    emit fontSizeChanged(size);
}

void EditorWidget::contextMenuEvent(QContextMenuEvent* event)
{
    QMenu* menu = new QMenu(this);

    // Font size submenu
    QMenu* fontSizeMenu = menu->addMenu(tr("Font Size"));
    for (int size = 10; size <= 20; ++size) {
        QAction* action = fontSizeMenu->addAction(QString::number(size));
        action->setCheckable(true);
        action->setChecked(m_fontSize == size);
        connect(action, &QAction::triggered, this, [this, size]() { setFontSize(size); });
    }
    menu->addSeparator();

    // Formatting actions
    QAction* boldAction = menu->addAction(tr("Bold"));
    boldAction->setShortcut(QKeySequence::Bold);
    connect(boldAction, &QAction::triggered, this, &EditorWidget::insertBold);

    QAction* italicAction = menu->addAction(tr("Italic"));
    italicAction->setShortcut(QKeySequence::Italic);
    connect(italicAction, &QAction::triggered, this, &EditorWidget::insertItalic);
    menu->addSeparator();

    QMenu* headingMenu = menu->addMenu(tr("Heading"));
    QAction* h1Action = headingMenu->addAction(tr("Heading 1"));
    connect(h1Action, &QAction::triggered, this, [this]() { insertHeading(1); });
    QAction* h2Action = headingMenu->addAction(tr("Heading 2"));
    connect(h2Action, &QAction::triggered, this, [this]() { insertHeading(2); });
    QAction* h3Action = headingMenu->addAction(tr("Heading 3"));
    connect(h3Action, &QAction::triggered, this, [this]() { insertHeading(3); });

    QAction* bulletAction = menu->addAction(tr("Bullet List"));
    connect(bulletAction, &QAction::triggered, this, &EditorWidget::insertBulletList);

    QAction* numberedAction = menu->addAction(tr("Numbered List"));
    connect(numberedAction, &QAction::triggered, this, &EditorWidget::insertNumberedList);
    menu->addSeparator();

    QAction* codeAction = menu->addAction(tr("Code Block"));
    connect(codeAction, &QAction::triggered, this, &EditorWidget::insertCodeBlock);

    QAction* linkAction = menu->addAction(tr("Insert Link"));
    connect(linkAction, &QAction::triggered, this, &EditorWidget::insertLink);

    QAction* wikilinkAction = menu->addAction(tr("Insert Wikilink"));
    connect(wikilinkAction, &QAction::triggered, this, &EditorWidget::insertWikilink);
    menu->addSeparator();

    // Standard editor actions
    QAction* undoAction = menu->addAction(tr("Undo"));
    undoAction->setShortcut(QKeySequence::Undo);
    connect(undoAction, &QAction::triggered, this, &QPlainTextEdit::undo);

    QAction* redoAction = menu->addAction(tr("Redo"));
    redoAction->setShortcut(QKeySequence::Redo);
    connect(redoAction, &QAction::triggered, this, &QPlainTextEdit::redo);
    menu->addSeparator();

    QAction* cutAction = menu->addAction(tr("Cut"));
    cutAction->setShortcut(QKeySequence::Cut);
    connect(cutAction, &QAction::triggered, this, &QPlainTextEdit::cut);

    QAction* copyAction = menu->addAction(tr("Copy"));
    copyAction->setShortcut(QKeySequence::Copy);
    connect(copyAction, &QAction::triggered, this, &QPlainTextEdit::copy);

    QAction* pasteAction = menu->addAction(tr("Paste"));
    pasteAction->setShortcut(QKeySequence::Paste);
    connect(pasteAction, &QAction::triggered, this, &QPlainTextEdit::paste);

    menu->exec(event->globalPos());
    delete menu;
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
    if (!m_lineNumberArea->isVisible())
        return 0;

    int digits = 1;
    int max = std::max(1, blockCount());
    while (max >= 10) { max /= 10; ++digits; }

    int space = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
    return space;
}

void EditorWidget::lineNumberAreaPaintEvent(QPaintEvent* event)
{
    QPainter painter(m_lineNumberArea);
    painter.setFont(font());
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
