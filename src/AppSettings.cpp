#include "AppSettings.h"

AppSettings::AppSettings()
    : m_settings("Durandal", "Durandal")
    , m_useDarkTheme(false)
    , m_editorFontSize(13)
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
