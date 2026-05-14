#ifndef MARKDOWNRENDERER_H
#define MARKDOWNRENDERER_H

#include <QString>
#include <QUrl>
#include <functional>

class VaultManager;

class MarkdownRenderer
{
public:
    MarkdownRenderer();

    QString renderToHtml(const QString& markdown) const;
    QString wrapInDocument(const QString& bodyHtml, bool dark) const;
    QString renderFull(const QString& markdown, bool dark) const;

private:
    QString preprocessWikilinks(const QString& markdown) const;
    QString postprocessCodeBlocks(const QString& html) const;
};

#endif // MARKDOWNRENDERER_H
