#include "SearchEngine.h"
#include <QFileInfo>
#include <QRegularExpression>
#include <algorithm>

SearchEngine::SearchEngine(QObject* parent)
    : QObject(parent)
{
}

void SearchEngine::indexFiles(const QMap<QString, QString>& fileContents)
{
    m_index = fileContents;
}

void SearchEngine::clear()
{
    m_index.clear();
}

QList<SearchResult> SearchEngine::search(const QString& query, int maxResults) const
{
    QList<SearchResult> results;

    if (query.trimmed().isEmpty()) return results;

    QStringList terms = query.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);

    for (auto it = m_index.constBegin(); it != m_index.constEnd(); ++it) {
        SearchResult result;
        result.filePath = it.key();
        QFileInfo fi(result.filePath);
        result.fileName = fi.fileName();
        result.score = computeScore(fi.completeBaseName(), it.value(), query);
        result.preview.clear();

        if (result.score > 0) {
            QString contentLower = it.value().toLower();
            QString queryLower = query.toLower();

            int idx = contentLower.indexOf(queryLower);
            if (idx >= 0) {
                int start = std::max(0, idx - 40);
                int end = std::min(contentLower.size(), idx + query.size() + 60);
                result.preview = it.value().mid(start, end - start).trimmed();

                if (start > 0) result.preview = "..." + result.preview;
                if (end < it.value().size()) result.preview = result.preview + "...";
            }

            results.append(result);
        }
    }

    std::sort(results.begin(), results.end(), [](const SearchResult& a, const SearchResult& b) {
        if (a.score != b.score) return a.score > b.score;
        return a.fileName < b.fileName;
    });

    if (results.size() > maxResults) {
        results = results.mid(0, maxResults);
    }

    return results;
}

int SearchEngine::computeScore(const QString& fileName, const QString& content,
                                const QString& query) const
{
    Q_UNUSED(content)
    QString fnLower = fileName.toLower();
    QString qLower = query.toLower();

    int score = 0;

    if (fnLower == qLower) {
        score += 100;
    } else if (fnLower.startsWith(qLower)) {
        score += 80;
    } else if (fnLower.contains(qLower)) {
        score += 60;
    }

    QStringList terms = query.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    for (const QString& term : terms) {
        if (fnLower.contains(term.toLower())) {
            score += 20;
        }
    }

    QString contentLower = content.toLower();
    if (contentLower.contains(qLower)) {
        score += 40;
        int count = contentLower.count(qLower);
        score += std::min(count, 10);
    }

    if (score == 0 && (fnLower.contains(qLower) || contentLower.contains(qLower))) {
        score = 1;
    }

    return score;
}
