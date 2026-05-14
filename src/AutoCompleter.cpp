#include "AutoCompleter.h"
#include <QTextBlock>
#include <QTextCursor>
#include <QScrollBar>
#include <QAbstractItemView>

AutoCompleter::AutoCompleter(QPlainTextEdit* editor, QObject* parent)
    : QObject(parent)
    , m_editor(editor)
    , m_completer(new QCompleter(this))
    , m_model(new QStringListModel(this))
    , m_completing(false)
{
    m_completer->setModel(m_model);
    m_completer->setCompletionMode(QCompleter::PopupCompletion);
    m_completer->setCaseSensitivity(Qt::CaseInsensitive);
    m_completer->setFilterMode(Qt::MatchContains);

    connect(m_editor, &QPlainTextEdit::textChanged,
            this, &AutoCompleter::onTextChanged);
    connect(m_completer, QOverload<const QString&>::of(&QCompleter::activated),
            this, &AutoCompleter::onCompletionActivated);
}

void AutoCompleter::setWikilinks(const QStringList& links)
{
    m_wikilinks = links;
    m_wikilinks.sort(Qt::CaseInsensitive);
}

void AutoCompleter::setTags(const QStringList& tags)
{
    m_tags = tags;
    m_tags.sort(Qt::CaseInsensitive);
}

void AutoCompleter::onTextChanged()
{
    if (m_completing) return;

    QTextCursor cursor = m_editor->textCursor();
    int pos = cursor.position();

    Context ctx = detectContext(pos);
    if (ctx == None) {
        m_completer->popup()->hide();
        return;
    }

    QString prefix = currentPrefix(pos, ctx);
    if (prefix.isEmpty()) {
        m_completer->popup()->hide();
        return;
    }

    QStringList candidates = (ctx == Wikilink) ? m_wikilinks : m_tags;
    showCompletions(candidates, prefix);
}

void AutoCompleter::onCompletionActivated(const QString& text)
{
    if (m_completing) return;
    m_completing = true;

    QTextCursor cursor = m_editor->textCursor();
    int pos = cursor.position();
    Context ctx = detectContext(pos);
    QString prefix = currentPrefix(pos, ctx);

    cursor.beginEditBlock();

    if (ctx == Wikilink) {
        int triggerPos = pos - prefix.length() - 2; // -2 for "[["
        cursor.setPosition(triggerPos);
        cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor,
                           prefix.length() + 2);
        cursor.insertText(QStringLiteral("[[%1]]").arg(text));
    } else if (ctx == Tag) {
        int triggerPos = pos - prefix.length() - 1; // -1 for "#"
        cursor.setPosition(triggerPos);
        cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor,
                           prefix.length() + 1);
        cursor.insertText(text);
    }

    cursor.endEditBlock();
    m_editor->setTextCursor(cursor);
    m_completer->popup()->hide();
    m_completing = false;
}

AutoCompleter::Context AutoCompleter::detectContext(int cursorPos) const
{
    QString text = m_editor->toPlainText();
    if (text.isEmpty()) return None;

    int start = std::max(0, cursorPos - 500);
    QChar prev;

    for (int i = cursorPos - 1; i >= start; --i) {
        QChar ch = text.at(i);

        if (ch == QChar::LineFeed || ch == QChar::ParagraphSeparator) {
            return None;
        }

        if (ch == ']' && i > 0 && text.at(i - 1) == ']') {
            // Past a closing ]], check if there was an opening
            for (int j = i - 2; j >= start; --j) {
                if (text.at(j) == '[' && j > 0 && text.at(j - 1) == '[') {
                    // We're inside a wikilink
                    if (cursorPos - 1 > j - 1) {
                        return Wikilink;
                    }
                }
            }
            return None;
        }

        if (ch == '[' && i > 0 && text.at(i - 1) == '[') {
            return Wikilink;
        }

        if (ch == '#') {
            if (i == 0 || text.at(i - 1).isSpace() || text.at(i - 1) == QChar::LineFeed) {
                return Tag;
            }
            return None;
        }
    }

    return None;
}

QString AutoCompleter::currentPrefix(int cursorPos, Context ctx) const
{
    QString text = m_editor->toPlainText();
    int start = cursorPos - 1;
    QString prefix;

    while (start >= 0) {
        QChar ch = text.at(start);
        if (ch == QChar::LineFeed || ch == QChar::ParagraphSeparator) break;
        if (ctx == Wikilink && ch == '[' && start > 0 && text.at(start - 1) == '[') break;
        if (ctx == Tag && ch == '#') break;
        if (ctx == Tag && ch.isSpace()) break;
        prefix.prepend(ch);
        --start;
    }

    return prefix;
}

void AutoCompleter::showCompletions(const QStringList& items, const QString& prefix)
{
    QStringList filtered;
    for (const QString& item : items) {
        if (item.contains(prefix, Qt::CaseInsensitive)) {
            filtered.append(item);
        }
    }

    if (filtered.isEmpty()) {
        m_completer->popup()->hide();
        return;
    }

    m_model->setStringList(filtered);
    m_completer->setCompletionPrefix(prefix);

    QRect cursorRect = m_editor->cursorRect();
    cursorRect.setWidth(m_completer->popup()->sizeHintForColumn(0)
                        + m_completer->popup()->verticalScrollBar()->sizeHint().width() + 30);

    m_completer->complete(cursorRect);
}
