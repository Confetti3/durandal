#ifndef WIKILINKPARSER_H
#define WIKILINKPARSER_H

#include <QString>
#include <QStringList>
#include <QMap>
#include <QRegularExpression>

class WikilinkParser
{
public:
    struct LinkInfo {
        QString title;      // Raw [[title]]
        int start;          // Position in text
        int end;
    };

    WikilinkParser();

    QList<LinkInfo> parseLinks(const QString& text) const;
    QStringList extractUniqueTitles(const QString& text) const;

    // Builds a map: note file path -> list of files that link to it
    QMap<QString, QStringList> buildBacklinksIndex(
        const QMap<QString, QString>& fileContents) const;

    // Get all notes that link to a given note title
    QStringList findBacklinks(const QString& targetTitle,
                              const QMap<QString, QString>& fileContents) const;

private:
    QRegularExpression m_wikilinkRe;
};

#endif // WIKILINKPARSER_H
