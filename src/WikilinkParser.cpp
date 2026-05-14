#include "WikilinkParser.h"
#include <QRegularExpressionMatch>
#include <QFileInfo>

WikilinkParser::WikilinkParser()
    : m_wikilinkRe(R"(\[\[([^\]]+)\]\])")
{
}

QList<WikilinkParser::LinkInfo> WikilinkParser::parseLinks(const QString& text) const
{
    QList<LinkInfo> links;
    QRegularExpressionMatchIterator it = m_wikilinkRe.globalMatch(text);

    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        LinkInfo info;
        info.title = match.captured(1).trimmed();
        info.start = match.capturedStart();
        info.end = match.capturedEnd();
        links.append(info);
    }
    return links;
}

QStringList WikilinkParser::extractUniqueTitles(const QString& text) const
{
    QStringList titles;
    QSet<QString> seen;
    QRegularExpressionMatchIterator it = m_wikilinkRe.globalMatch(text);

    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString title = match.captured(1).trimmed();
        if (!title.isEmpty() && !seen.contains(title.toLower())) {
            seen.insert(title.toLower());
            titles.append(title);
        }
    }
    return titles;
}

QMap<QString, QStringList> WikilinkParser::buildBacklinksIndex(
    const QMap<QString, QString>& fileContents) const
{
    QMap<QString, QStringList> index;

    for (auto it = fileContents.constBegin(); it != fileContents.constEnd(); ++it) {
        QString sourceFile = it.key();
        QFileInfo fi(sourceFile);
        QString sourceTitle = fi.completeBaseName();

        QStringList linkedTitles = extractUniqueTitles(it.value());

        for (const QString& targetTitle : linkedTitles) {
            if (targetTitle.compare(sourceTitle, Qt::CaseInsensitive) != 0) {
                index[targetTitle.toLower()].append(sourceFile);
            }
        }
    }

    for (auto& list : index) {
        list.removeDuplicates();
    }

    return index;
}

QStringList WikilinkParser::findBacklinks(const QString& targetTitle,
                                           const QMap<QString, QString>& fileContents) const
{
    QStringList backlinks;
    QFileInfo targetFi(targetTitle);
    QString targetBase = targetFi.completeBaseName();

    for (auto it = fileContents.constBegin(); it != fileContents.constEnd(); ++it) {
        QString sourceFile = it.key();
        QFileInfo sourceFi(sourceFile);
        QString sourceBase = sourceFi.completeBaseName();

        if (sourceBase.compare(targetBase, Qt::CaseInsensitive) == 0) continue;

        QStringList linked = extractUniqueTitles(it.value());
        for (const QString& link : linked) {
            if (link.compare(targetBase, Qt::CaseInsensitive) == 0) {
                backlinks.append(sourceFile);
                break;
            }
        }
    }
    return backlinks;
}
