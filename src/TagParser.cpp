#include "TagParser.h"
#include <QRegularExpressionMatch>

TagParser::TagParser()
    : m_tagRe(R"((?:^|\s)(#[^\s#.,;:!?()[\]{}<>]+))")
{
}

QStringList TagParser::extractTags(const QString& text) const
{
    QStringList tags;
    QRegularExpressionMatchIterator it = m_tagRe.globalMatch(text);

    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString tag = match.captured(1).trimmed();
        if (!tag.isEmpty()) {
            tags.append(tag);
        }
    }
    return tags;
}

QSet<QString> TagParser::extractUniqueTags(const QString& text) const
{
    QSet<QString> unique;
    QStringList all = extractTags(text);
    for (const QString& tag : all) {
        unique.insert(tag.toLower());
    }
    return unique;
}

QMap<QString, QStringList> TagParser::buildTagIndex(
    const QMap<QString, QString>& fileContents) const
{
    QMap<QString, QStringList> index;

    for (auto it = fileContents.constBegin(); it != fileContents.constEnd(); ++it) {
        QString filePath = it.key();
        QSet<QString> tags = extractUniqueTags(it.value());

        for (const QString& tag : tags) {
            index[tag].append(filePath);
        }
    }

    for (auto& list : index) {
        list.removeDuplicates();
    }

    return index;
}

QStringList TagParser::findFilesByTag(const QString& tag,
                                       const QMap<QString, QString>& fileContents) const
{
    QStringList result;
    QString tagLower = "#" + tag.toLower();

    for (auto it = fileContents.constBegin(); it != fileContents.constEnd(); ++it) {
        QSet<QString> tags = extractUniqueTags(it.value());
        if (tags.contains(tag.toLower()) || tags.contains(tagLower)) {
            result.append(it.key());
        }
    }
    return result;
}
