#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <QString>
#include <QStringList>
#include <QSettings>
#include <QByteArray>

class AppSettings
{
public:
    AppSettings();

    void load();
    void save();

    QString lastFolderPath() const;
    void setLastFolderPath(const QString& path);

    QStringList recentFolders() const;
    void setRecentFolders(const QStringList& folders);
    void addRecentFolder(const QString& path);

    bool useDarkTheme() const;
    void setUseDarkTheme(bool dark);

    QByteArray windowGeometry() const;
    void setWindowGeometry(const QByteArray& geometry);

    QByteArray windowState() const;
    void setWindowState(const QByteArray& state);

    QByteArray splitterState() const;
    void setSplitterState(const QByteArray& state);

    int editorFontSize() const;
    void setEditorFontSize(int size);

    bool editorWordWrap() const;
    void setEditorWordWrap(bool wrap);

    // New settings
    QString editorFontFamily() const;
    void setEditorFontFamily(const QString& family);

    int editorTabWidth() const;
    void setEditorTabWidth(int spaces);

    bool editorShowLineNumbers() const;
    void setEditorShowLineNumbers(bool show);

    bool autoSaveEnabled() const;
    void setAutoSaveEnabled(bool enabled);

    int autoSaveInterval() const;
    void setAutoSaveInterval(int seconds);

    int previewUpdateDelay() const;
    void setPreviewUpdateDelay(int ms);

    bool openLastFolderOnStartup() const;
    void setOpenLastFolderOnStartup(bool open);

private:
    QSettings m_settings;
    QString m_lastFolderPath;
    QStringList m_recentFolders;
    bool m_useDarkTheme;
    QByteArray m_windowGeometry;
    QByteArray m_windowState;
    QByteArray m_splitterState;
    int m_editorFontSize;
    bool m_editorWordWrap;

    // New fields
    QString m_editorFontFamily;
    int m_editorTabWidth;
    bool m_editorShowLineNumbers;
    bool m_autoSaveEnabled;
    int m_autoSaveInterval;
    int m_previewUpdateDelay;
    bool m_openLastFolderOnStartup;

    static constexpr int MAX_RECENT_FOLDERS = 10;
};

#endif // APPSETTINGS_H
