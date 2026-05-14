#ifndef AUTOCOMPLETER_H
#define AUTOCOMPLETER_H

#include <QObject>
#include <QPlainTextEdit>
#include <QCompleter>
#include <QStringListModel>

class AutoCompleter : public QObject
{
    Q_OBJECT

public:
    explicit AutoCompleter(QPlainTextEdit* editor, QObject* parent = nullptr);

    void setWikilinks(const QStringList& links);
    void setTags(const QStringList& tags);

private slots:
    void onTextChanged();
    void onCompletionActivated(const QString& text);

private:
    QPlainTextEdit* m_editor;
    QCompleter* m_completer;
    QStringListModel* m_model;
    QStringList m_wikilinks;
    QStringList m_tags;
    bool m_completing;

    enum Context { None, Wikilink, Tag };
    Context detectContext(int cursorPos) const;
    QString currentPrefix(int cursorPos, Context ctx) const;
    void showCompletions(const QStringList& items, const QString& prefix);
};

#endif // AUTOCOMPLETER_H
