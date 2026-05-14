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

private:
    QSettings m_settings;
    QString m_lastFolderPath;
    QStringList m_recentFolders;
    bool m_useDarkTheme;
    QByteArray m_windowGeometry;
    QByteArray m_windowState;
    QByteArray m_splitterState;

    static constexpr int MAX_RECENT_FOLDERS = 10;
};

#endif // APPSETTINGS_H
