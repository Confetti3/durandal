#include "AppSettings.h"

AppSettings::AppSettings()
    : m_settings("Durandal", "Durandal")
    , m_useDarkTheme(false)
    , m_editorFontSize(13)
    , m_editorWordWrap(false)
    , m_editorFontFamily("Consolas")
    , m_editorTabWidth(4)
    , m_editorShowLineNumbers(true)
    , m_autoSaveEnabled(true)
    , m_autoSaveInterval(30)
    , m_previewUpdateDelay(300)
    , m_openLastFolderOnStartup(false)
{
}

void AppSettings::load()
{
    m_lastFolderPath = m_settings.value("folder/lastPath").toString();
    m_recentFolders = m_settings.value("folder/recent").toStringList();
    m_useDarkTheme = m_settings.value("theme/dark", false).toBool();
    m_windowGeometry = m_settings.value("window/geometry").toByteArray();
    m_windowState = m_settings.value("window/state").toByteArray();
    m_splitterState = m_settings.value("window/splitterState").toByteArray();
    m_editorFontSize = m_settings.value("editor/fontSize", 13).toInt();
    m_editorWordWrap = m_settings.value("editor/wordWrap", false).toBool();

    m_editorFontFamily = m_settings.value("editor/fontFamily", "Consolas").toString();
    m_editorTabWidth = m_settings.value("editor/tabWidth", 4).toInt();
    m_editorShowLineNumbers = m_settings.value("editor/showLineNumbers", true).toBool();
    m_autoSaveEnabled = m_settings.value("autoSave/enabled", true).toBool();
    m_autoSaveInterval = m_settings.value("autoSave/interval", 30).toInt();
    m_previewUpdateDelay = m_settings.value("preview/updateDelay", 300).toInt();
    m_openLastFolderOnStartup = m_settings.value("general/openLastFolder", false).toBool();
}

void AppSettings::save()
{
    m_settings.setValue("folder/lastPath", m_lastFolderPath);
    m_settings.setValue("folder/recent", m_recentFolders);
    m_settings.setValue("theme/dark", m_useDarkTheme);
    m_settings.setValue("window/geometry", m_windowGeometry);
    m_settings.setValue("window/state", m_windowState);
    m_settings.setValue("window/splitterState", m_splitterState);
    m_settings.setValue("editor/fontSize", m_editorFontSize);
    m_settings.setValue("editor/wordWrap", m_editorWordWrap);

    m_settings.setValue("editor/fontFamily", m_editorFontFamily);
    m_settings.setValue("editor/tabWidth", m_editorTabWidth);
    m_settings.setValue("editor/showLineNumbers", m_editorShowLineNumbers);
    m_settings.setValue("autoSave/enabled", m_autoSaveEnabled);
    m_settings.setValue("autoSave/interval", m_autoSaveInterval);
    m_settings.setValue("preview/updateDelay", m_previewUpdateDelay);
    m_settings.setValue("general/openLastFolder", m_openLastFolderOnStartup);
    m_settings.sync();
}

QString AppSettings::lastFolderPath() const { return m_lastFolderPath; }
void AppSettings::setLastFolderPath(const QString& path) { m_lastFolderPath = path; }

QStringList AppSettings::recentFolders() const { return m_recentFolders; }
void AppSettings::setRecentFolders(const QStringList& folders) { m_recentFolders = folders; }

void AppSettings::addRecentFolder(const QString& path)
{
    if (path.isEmpty()) return;
    m_recentFolders.removeAll(path);
    m_recentFolders.prepend(path);
    while (m_recentFolders.size() > MAX_RECENT_FOLDERS)
        m_recentFolders.removeLast();
}

bool AppSettings::useDarkTheme() const { return m_useDarkTheme; }
void AppSettings::setUseDarkTheme(bool dark) { m_useDarkTheme = dark; }

QByteArray AppSettings::windowGeometry() const { return m_windowGeometry; }
void AppSettings::setWindowGeometry(const QByteArray& geometry) { m_windowGeometry = geometry; }

QByteArray AppSettings::windowState() const { return m_windowState; }
void AppSettings::setWindowState(const QByteArray& state) { m_windowState = state; }

QByteArray AppSettings::splitterState() const { return m_splitterState; }
void AppSettings::setSplitterState(const QByteArray& state) { m_splitterState = state; }

int AppSettings::editorFontSize() const { return m_editorFontSize; }
void AppSettings::setEditorFontSize(int size) { m_editorFontSize = size; }

bool AppSettings::editorWordWrap() const { return m_editorWordWrap; }
void AppSettings::setEditorWordWrap(bool wrap) { m_editorWordWrap = wrap; }

QString AppSettings::editorFontFamily() const { return m_editorFontFamily; }
void AppSettings::setEditorFontFamily(const QString& family) { m_editorFontFamily = family; }

int AppSettings::editorTabWidth() const { return m_editorTabWidth; }
void AppSettings::setEditorTabWidth(int spaces) { m_editorTabWidth = spaces; }

bool AppSettings::editorShowLineNumbers() const { return m_editorShowLineNumbers; }
void AppSettings::setEditorShowLineNumbers(bool show) { m_editorShowLineNumbers = show; }

bool AppSettings::autoSaveEnabled() const { return m_autoSaveEnabled; }
void AppSettings::setAutoSaveEnabled(bool enabled) { m_autoSaveEnabled = enabled; }

int AppSettings::autoSaveInterval() const { return m_autoSaveInterval; }
void AppSettings::setAutoSaveInterval(int seconds) { m_autoSaveInterval = seconds; }

int AppSettings::previewUpdateDelay() const { return m_previewUpdateDelay; }
void AppSettings::setPreviewUpdateDelay(int ms) { m_previewUpdateDelay = ms; }

bool AppSettings::openLastFolderOnStartup() const { return m_openLastFolderOnStartup; }
void AppSettings::setOpenLastFolderOnStartup(bool open) { m_openLastFolderOnStartup = open; }
