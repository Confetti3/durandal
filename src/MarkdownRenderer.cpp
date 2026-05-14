#include "MarkdownRenderer.h"
#include <cmark.h>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

MarkdownRenderer::MarkdownRenderer()
{
}

QString MarkdownRenderer::renderToHtml(const QString& markdown) const
{
    QString processed = preprocessWikilinks(markdown);
    QByteArray input = processed.toUtf8();

    char* rawHtml = cmark_markdown_to_html(input.constData(), input.size(),
                                           CMARK_OPT_UNSAFE);
    QString html = QString::fromUtf8(rawHtml);
    free(rawHtml);

    return postprocessCodeBlocks(html);
}

QString MarkdownRenderer::wrapInDocument(const QString& bodyHtml, bool dark) const
{
    // Terminal aesthetic colors
    QString bgColor = dark ? "#0c0c0c" : "#ffffff";
    QString textColor = dark ? "#cccccc" : "#1a1a1a";
    QString codeBg = dark ? "#161616" : "#f5f5f5";
    QString codeColor = dark ? "#0dbc79" : "#007acc";
    QString headingColor = dark ? "#cccccc" : "#1a1a1a";
    QString linkColor = dark ? "#0dbc79" : "#0056b3";
    QString borderColor = dark ? "#2b2b2b" : "#d0d0d0";
    QString blockBg = dark ? "#161616" : "#f5f5f5";
    QString tableBorder = dark ? "#2b2b2b" : "#d0d0d0";
    QString tableAlt = dark ? "#111111" : "#fafafa";
    QString headingBorder = dark ? "#3d3d3d" : "#d0d0d0";

    return QStringLiteral(R"(
<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<style>
body {
    font-family: "Cascadia Code", "Consolas", "Courier New", monospace;
    font-size: 14px;
    line-height: 1.6;
    color: %2;
    background-color: %1;
    max-width: 880px;
    margin: 0 auto;
    padding: 24px;
}
h1, h2, h3, h4, h5, h6 {
    color: %5;
    margin-top: 1.6em;
    margin-bottom: 0.6em;
    font-weight: 600;
    font-family: "Cascadia Code", "Consolas", "Courier New", monospace;
}
h1 { font-size: 1.8em; border-bottom: 1px solid %11; padding-bottom: 0.3em; }
h2 { font-size: 1.5em; border-bottom: 1px solid %11; padding-bottom: 0.25em; }
h3 { font-size: 1.25em; }
h4 { font-size: 1.1em; }
h5 { font-size: 1em; }
h6 { font-size: 0.95em; color: #6e7681; }
a { color: %6; text-decoration: none; border-bottom: 1px dotted %6; }
a:hover { text-decoration: none; border-bottom: 1px solid %6; }
pre {
    background-color: %3;
    border: 1px solid %7;
    border-radius: 0px;
    padding: 14px;
    overflow-x: auto;
    font-family: "Cascadia Code", "Consolas", "Courier New", monospace;
    font-size: 13px;
    line-height: 1.5;
}
code {
    font-family: "Cascadia Code", "Consolas", "Courier New", monospace;
    font-size: 13px;
    background-color: %3;
    color: %4;
    padding: 2px 5px;
    border: 1px solid %7;
    border-radius: 0px;
}
pre code {
    background-color: transparent;
    border: none;
    padding: 0;
    color: %2;
}
blockquote {
    border-left: 3px solid %6;
    margin: 0;
    padding: 4px 16px;
    background-color: %8;
    color: %2;
}
table {
    border-collapse: collapse;
    width: 100%%;
    margin: 16px 0;
    font-size: 13px;
}
th, td {
    border: 1px solid %9;
    padding: 8px 10px;
    text-align: left;
}
th {
    background-color: %3;
    font-weight: 600;
    color: %2;
}
tr:nth-child(even) { background-color: %10; }
img { max-width: 100%%; border: 1px solid %7; }
hr { border: none; border-top: 1px solid %7; margin: 24px 0; }
ul, ol { padding-left: 24px; }
li { margin: 4px 0; }
p { margin: 0.9em 0; }
strong { color: %2; font-weight: 600; }
em { color: %2; font-style: italic; }
del { color: #6e7681; text-decoration: line-through; }
    </style>
</head>
<body>
%12
</body>
</html>
)")
        .arg(bgColor, textColor, codeBg, codeColor,
             headingColor, linkColor, borderColor,
             blockBg, tableBorder, tableAlt, headingBorder, bodyHtml);
}

QString MarkdownRenderer::renderFull(const QString& markdown, bool dark) const
{
    QString html = renderToHtml(markdown);
    return wrapInDocument(html, dark);
}

QString MarkdownRenderer::preprocessWikilinks(const QString& markdown) const
{
    static const QRegularExpression wikilinkRe(R"(\[\[([^\]]+)\]\])");
    QString result = markdown;
    QRegularExpressionMatchIterator it = wikilinkRe.globalMatch(markdown);

    QList<QPair<int, int>> replacements;
    QList<QString> replacementTexts;

    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString title = match.captured(1);
        QString linkText = QStringLiteral("[%1](wikilink:%1)").arg(title);
        replacements.append({match.capturedStart(), match.capturedLength()});
        replacementTexts.append(linkText);
    }

    for (int i = replacements.size() - 1; i >= 0; --i) {
        result.replace(replacements[i].first, replacements[i].second, replacementTexts[i]);
    }

    return result;
}

QString MarkdownRenderer::postprocessCodeBlocks(const QString& html) const
{
    return html;
}
