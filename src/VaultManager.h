#ifndef VAULTMANAGER_H
#define VAULTMANAGER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QFileSystemWatcher>
#include <QDir>

class VaultManager : public QObject
{
    Q_OBJECT

public:
    explicit VaultManager(QObject* parent = nullptr);

    void openFolder(const QString& path);
    void closeFolder();
    bool isOpen() const;

    QString folderPath() const;
    QString folderName() const;
    QStringList allFiles() const;
    QStringList allNoteTitles() const;

    QString readFile(const QString& relativePath) const;
    bool writeFile(const QString& relativePath, const QString& content);
    bool createFile(const QString& parentDir, const QString& name);
    bool renameFile(const QString& relativePath, const QString& newName);
    bool deleteFile(const QString& relativePath);
    bool moveFile(const QString& relativePath, const QString& newParentDir);
    bool createDirectory(const QString& relativePath);

    QString fullPath(const QString& relativePath) const;
    QString relativePath(const QString& fullPath) const;
    QString resolveWikilink(const QString& title) const;

signals:
    void folderOpened(const QString& path);
    void folderClosed();
    void folderChanged();
    void fileCreated(const QString& path);
    void fileDeleted(const QString& path);
    void fileRenamed(const QString& oldPath, const QString& newPath);
    void fileMoved(const QString& oldPath, const QString& newPath);
    void fileChanged(const QString& path);

private slots:
    void onDirectoryChanged(const QString& path);
    void onFileChanged(const QString& path);

private:
    QString m_folderPath;
    QFileSystemWatcher* m_watcher;
    QStringList m_allFiles;

    void scanFolder();
    void addWatchPaths();
    QString readFileContent(const QString& path) const;
};

#endif // VAULTMANAGER_H
