#ifndef SEARCHENGINE_H
#define SEARCHENGINE_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QMap>

struct SearchResult {
    QString filePath;
    QString fileName;
    QString preview;
    int score;
};

class SearchEngine : public QObject
{
    Q_OBJECT

public:
    explicit SearchEngine(QObject* parent = nullptr);

    void indexFiles(const QMap<QString, QString>& fileContents);
    void clear();

    QList<SearchResult> search(const QString& query, int maxResults = 50) const;

private:
    QMap<QString, QString> m_index;
    int computeScore(const QString& fileName, const QString& content,
                     const QString& query) const;
};

#endif // SEARCHENGINE_H
