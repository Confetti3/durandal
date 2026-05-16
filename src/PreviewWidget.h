#ifndef PREVIEWWIDGET_H
#define PREVIEWWIDGET_H

#include <QWidget>
#include <QWebEngineView>
#include <QWebEnginePage>
#include <QUrl>

class MarkdownRenderer;

class PreviewPage : public QWebEnginePage
{
    Q_OBJECT
public:
    explicit PreviewPage(QObject* parent = nullptr);

signals:
    void wikilinkClicked(const QString& title);
    void externalLinkClicked(const QUrl& url);

protected:
    bool acceptNavigationRequest(const QUrl& url, NavigationType type, bool isMainFrame) override;
};

class PreviewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PreviewWidget(QWidget* parent = nullptr);

    void setMarkdown(const QString& markdown, bool dark);
    void setHtmlContent(const QString& html, bool dark);
    void setBaseUrl(const QUrl& url);
    void setTheme(bool dark);
    void refresh();

signals:
    void wikilinkClicked(const QString& title);
    void externalLinkClicked(const QUrl& url);

private:
    void injectBodyHtml(const QString& bodyHtml);
    void saveScrollPosition();
    void restoreScrollPosition();

    QWebEngineView* m_webView;
    PreviewPage* m_page;
    MarkdownRenderer* m_renderer;
    QString m_currentMarkdown;
    bool m_dark;
    QUrl m_baseUrl;
    bool m_contentReady;
    int m_savedScrollY;
};

#endif // PREVIEWWIDGET_H
