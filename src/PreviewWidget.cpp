#include "PreviewWidget.h"
#include "MarkdownRenderer.h"
#include <QVBoxLayout>
#include <QDesktopServices>
#include <QMenu>
#include <QAction>
#include <QShortcut>
#include <QKeySequence>

PreviewPage::PreviewPage(QObject* parent)
    : QWebEnginePage(parent)
{
}

bool PreviewPage::acceptNavigationRequest(const QUrl& url, NavigationType type, bool isMainFrame)
{
    if (type == NavigationTypeLinkClicked) {
        if (url.scheme() == "wikilink") {
            QString title = url.path();
            if (title.isEmpty()) title = url.host().isEmpty() ? url.toString() : url.host();
            emit wikilinkClicked(title);
            return false;
        }
        if (url.scheme() == "http" || url.scheme() == "https") {
            emit externalLinkClicked(url);
            QDesktopServices::openUrl(url);
            return false;
        }
    }
    return QWebEnginePage::acceptNavigationRequest(url, type, isMainFrame);
}

PreviewWidget::PreviewWidget(QWidget* parent)
    : QWidget(parent)
    , m_webView(new QWebEngineView(this))
    , m_page(new PreviewPage(this))
    , m_renderer(new MarkdownRenderer)
    , m_dark(false)
    , m_contentReady(false)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_webView);

    m_webView->setPage(m_page);
    m_webView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_webView, &QWidget::customContextMenuRequested,
            this, [this](const QPoint& pos) {
                QMenu menu(m_webView);
                QAction* copyAction = menu.addAction(tr("Copy"));
                connect(copyAction, &QAction::triggered, this, [this]() {
                    m_page->triggerAction(QWebEnginePage::Copy);
                });
                QAction* selectAllAction = menu.addAction(tr("Select All"));
                connect(selectAllAction, &QAction::triggered, this, [this]() {
                    m_page->triggerAction(QWebEnginePage::SelectAll);
                });
                menu.exec(m_webView->mapToGlobal(pos));
            });
    m_page->setBackgroundColor(m_dark ? QColor("#1e1e2e") : Qt::white);

    // Keyboard shortcuts for the preview
    QShortcut* copyShortcut = new QShortcut(QKeySequence::Copy, m_webView);
    connect(copyShortcut, &QShortcut::activated,
            [this]() { m_page->triggerAction(QWebEnginePage::Copy); });
    QShortcut* selectAllShortcut = new QShortcut(QKeySequence::SelectAll, m_webView);
    connect(selectAllShortcut, &QShortcut::activated,
            [this]() { m_page->triggerAction(QWebEnginePage::SelectAll); });

    connect(m_page, &PreviewPage::wikilinkClicked,
            this, &PreviewWidget::wikilinkClicked);
    connect(m_page, &PreviewPage::externalLinkClicked,
            this, &PreviewWidget::externalLinkClicked);
    connect(m_webView, &QWebEngineView::loadFinished,
            this, [this](bool ok) { m_contentReady = ok; });
}

void PreviewWidget::setMarkdown(const QString& markdown, bool dark)
{
    m_currentMarkdown = markdown;
    bool themeChanged = (m_dark != dark);
    m_dark = dark;

    QString bodyHtml = m_renderer->renderToHtml(markdown);

    if (!m_contentReady || themeChanged) {
        m_webView->setHtml(m_renderer->wrapInDocument(bodyHtml, dark), m_baseUrl);
    } else {
        injectBodyHtml(bodyHtml);
    }
}

void PreviewWidget::setHtmlContent(const QString& html, bool dark)
{
    m_currentMarkdown = html;   // cached for refresh
    bool themeChanged = (m_dark != dark);
    m_dark = dark;

    if (!m_contentReady || themeChanged) {
        m_webView->setHtml(m_renderer->wrapInDocument(html, dark), m_baseUrl);
    } else {
        injectBodyHtml(html);
    }
}

void PreviewWidget::injectBodyHtml(const QString& bodyHtml)
{
    QByteArray utf8 = bodyHtml.toUtf8();
    QString base64 = QString::fromLatin1(utf8.toBase64());

    QString script = QStringLiteral(R"(
        (function() {
            var binary = atob('%1');
            var bytes = new Uint8Array(binary.length);
            for (var i = 0; i < binary.length; i++) {
                bytes[i] = binary.charCodeAt(i);
            }
            var decoded = new TextDecoder('utf-8').decode(bytes);
            document.body.innerHTML = decoded;
        })();
    )").arg(base64);

    m_page->runJavaScript(script);
}

void PreviewWidget::setBaseUrl(const QUrl& url)
{
    m_baseUrl = url;
}

void PreviewWidget::setTheme(bool dark)
{
    bool themeChanged = (m_dark != dark);
    m_dark = dark;
    m_page->setBackgroundColor(dark ? QColor("#1e1e2e") : Qt::white);
    if (themeChanged) {
        m_contentReady = false;
        refresh();
    }
}

void PreviewWidget::refresh()
{
    if (!m_currentMarkdown.isEmpty()) {
        setMarkdown(m_currentMarkdown, m_dark);
    }
}
