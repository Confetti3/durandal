#include "MarkdownHighlighter.h"

MarkdownHighlighter::MarkdownHighlighter(QTextDocument* parent)
    : QSyntaxHighlighter(parent)
{
    setupFormats();
    setupRules();
}

void MarkdownHighlighter::setupFormats()
{
    m_h1Format.setForeground(QColor("#1976d2"));
    m_h1Format.setFontWeight(QFont::Bold);
    m_h1Format.setFontPointSize(22);

    m_h2Format.setForeground(QColor("#2e7d32"));
    m_h2Format.setFontWeight(QFont::Bold);
    m_h2Format.setFontPointSize(18);

    m_h3Format.setForeground(QColor("#e65100"));
    m_h3Format.setFontWeight(QFont::Bold);
    m_h3Format.setFontPointSize(15);

    m_h4Format.setForeground(QColor("#6a1b9a"));
    m_h4Format.setFontWeight(QFont::Bold);
    m_h4Format.setFontPointSize(13);

    m_h5Format.setForeground(QColor("#00838f"));
    m_h5Format.setFontWeight(QFont::Bold);

    m_h6Format.setForeground(QColor("#c62828"));
    m_h6Format.setFontWeight(QFont::Bold);

    m_boldFormat.setFontWeight(QFont::Bold);
    m_boldFormat.setForeground(QColor("#333333"));

    m_italicFormat.setFontItalic(true);
    m_italicFormat.setForeground(QColor("#555555"));

    m_boldItalicFormat.setFontWeight(QFont::Bold);
    m_boldItalicFormat.setFontItalic(true);

    m_codeFormat.setFontFamily("Consolas");
    m_codeFormat.setFontFixedPitch(true);
    m_codeFormat.setBackground(QColor("#f5f5f5"));
    m_codeFormat.setForeground(QColor("#2e7d32"));

    m_inlineCodeFormat.setFontFamily("Consolas");
    m_inlineCodeFormat.setFontFixedPitch(true);
    m_inlineCodeFormat.setForeground(QColor("#c62828"));
    m_inlineCodeFormat.setBackground(QColor("#f5f5f5"));

    m_linkFormat.setForeground(QColor("#1976d2"));
    m_linkFormat.setFontUnderline(true);

    m_wikilinkFormat.setForeground(QColor("#00838f"));
    m_wikilinkFormat.setFontItalic(true);

    m_tagFormat.setForeground(QColor("#e65100"));
    m_tagFormat.setFontWeight(QFont::Bold);

    m_quoteFormat.setForeground(QColor("#6a1b9a"));
    m_quoteFormat.setFontItalic(true);

    m_hrFormat.setForeground(QColor("#bdbdbd"));

    m_listFormat.setForeground(QColor("#e65100"));
    m_listFormat.setFontWeight(QFont::Bold);

    m_htmlFormat.setForeground(QColor("#c62828"));
    m_htmlFormat.setFontFamily("Consolas");
}

void MarkdownHighlighter::setupRules()
{
    HighlightingRule rule;

    // Headings
    rule.pattern = QRegularExpression(R"(^#{6}\s+.*$)");
    rule.format = m_h6Format;
    m_rules.append(rule);

    rule.pattern = QRegularExpression(R"(^#{5}\s+.*$)");
    rule.format = m_h5Format;
    m_rules.append(rule);

    rule.pattern = QRegularExpression(R"(^#{4}\s+.*$)");
    rule.format = m_h4Format;
    m_rules.append(rule);

    rule.pattern = QRegularExpression(R"(^#{3}\s+.*$)");
    rule.format = m_h3Format;
    m_rules.append(rule);

    rule.pattern = QRegularExpression(R"(^#{2}\s+.*$)");
    rule.format = m_h2Format;
    m_rules.append(rule);

    rule.pattern = QRegularExpression(R"(^#{1}\s+.*$)");
    rule.format = m_h1Format;
    m_rules.append(rule);

    // Horizontal rules
    rule.pattern = QRegularExpression(R"(^[-*_]{3,}\s*$)");
    rule.format = m_hrFormat;
    m_rules.append(rule);

    // Block quotes
    rule.pattern = QRegularExpression(R"(^>\s+.*$)");
    rule.format = m_quoteFormat;
    m_rules.append(rule);

    // Unordered lists
    rule.pattern = QRegularExpression(R"(^\s*[-*+]\s+)");
    rule.format = m_listFormat;
    m_rules.append(rule);

    // Ordered lists
    rule.pattern = QRegularExpression(R"(^\s*\d+\.\s+)");
    rule.format = m_listFormat;
    m_rules.append(rule);

    // HTML tags
    rule.pattern = QRegularExpression(R"(</?[a-zA-Z0-9]+[^>]*>)");
    rule.format = m_htmlFormat;
    m_rules.append(rule);

    // Bold + Italic
    rule.pattern = QRegularExpression(R"(\*\*\*[^*]+\*\*\*)");
    rule.format = m_boldItalicFormat;
    m_rules.append(rule);

    // Bold
    rule.pattern = QRegularExpression(R"(\*\*[^*]+\*\*)");
    rule.format = m_boldFormat;
    m_rules.append(rule);

    rule.pattern = QRegularExpression(R"(__[^_]+__)");
    rule.format = m_boldFormat;
    m_rules.append(rule);

    // Italic
    rule.pattern = QRegularExpression(R"(\*[^*]+\*)");
    rule.format = m_italicFormat;
    m_rules.append(rule);

    rule.pattern = QRegularExpression(R"(_[^_]+_)");
    rule.format = m_italicFormat;
    m_rules.append(rule);

    // Wikilinks
    rule.pattern = QRegularExpression(R"(\[\[([^\]]+)\]\])");
    rule.format = m_wikilinkFormat;
    m_rules.append(rule);

    // Wiki-style links [text](url)
    rule.pattern = QRegularExpression(R"(\[([^\]]+)\]\(([^)]+)\))");
    rule.format = m_linkFormat;
    m_rules.append(rule);

    // Tags
    rule.pattern = QRegularExpression(R"((?:^|\s)(#[^\s#.,;:!?()[\]{}<>]+))");
    rule.format = m_tagFormat;
    m_rules.append(rule);

    // Inline code
    rule.pattern = QRegularExpression(R"(`[^`]+`)");
    rule.format = m_inlineCodeFormat;
    m_rules.append(rule);
}

void MarkdownHighlighter::highlightBlock(const QString& text)
{
    for (const HighlightingRule& rule : m_rules) {
        QRegularExpressionMatchIterator it = rule.pattern.globalMatch(text);
        while (it.hasNext()) {
            QRegularExpressionMatch match = it.next();

            if (match.capturedStart(1) >= 0) {
                setFormat(match.capturedStart(1), match.capturedLength(1), rule.format);
            } else {
                setFormat(match.capturedStart(), match.capturedLength(), rule.format);
            }
        }
    }

    // Highlight fenced code blocks
    setCurrentBlockState(0);

    static const QRegularExpression codeBlockStart(R"(^```\w*$)");
    static const QRegularExpression codeBlockEnd(R"(^```$)");

    QTextCharFormat codeFmt;
    codeFmt.setFontFamily("Consolas");
    codeFmt.setFontFixedPitch(true);
    codeFmt.setBackground(QColor("#f5f5f5"));
    codeFmt.setForeground(QColor("#2e7d32"));

    if (previousBlockState() == 1) {
        QRegularExpressionMatch endMatch = codeBlockEnd.match(text);
        if (endMatch.hasMatch()) {
            setFormat(0, text.length(), codeFmt);
            setCurrentBlockState(0);
        } else {
            setFormat(0, text.length(), codeFmt);
            setCurrentBlockState(1);
        }
    } else {
        QRegularExpressionMatch startMatch = codeBlockStart.match(text);
        if (startMatch.hasMatch()) {
            setFormat(0, text.length(), codeFmt);
            setCurrentBlockState(1);
        }
    }
}
