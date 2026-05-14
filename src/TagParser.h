#ifndef TAGPARSER_H
#define TAGPARSER_H

#include <QString>
#include <QStringList>
#include <QMap>
#include <QSet>
#include <QRegularExpression>

class TagParser
{
public:
    TagParser();

    QStringList extractTags(const QString& text) const;
    QSet<QString> extractUniqueTags(const QString& text) const;

    // Builds index: tag -> list of files containing that tag
    QMap<QString, QStringList> buildTagIndex(
        const QMap<QString, QString>& fileContents) const;

    // Get all files containing a specific tag
    QStringList findFilesByTag(const QString& tag,
                               const QMap<QString, QString>& fileContents) const;

private:
    QRegularExpression m_tagRe;
};

#endif // TAGPARSER_H
