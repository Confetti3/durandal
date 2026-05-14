#include "VaultManager.h"
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QRegularExpression>

VaultManager::VaultManager(QObject* parent)
    : QObject(parent)
    , m_watcher(new QFileSystemWatcher(this))
{
    connect(m_watcher, &QFileSystemWatcher::directoryChanged,
            this, &VaultManager::onDirectoryChanged);
    connect(m_watcher, &QFileSystemWatcher::fileChanged,
            this, &VaultManager::onFileChanged);
}

void VaultManager::openFolder(const QString& path)
{
    QDir dir(path);
    if (!dir.exists()) return;

    m_folderPath = dir.absolutePath();
    scanFolder();
    addWatchPaths();
    emit folderOpened(m_folderPath);
}

void VaultManager::closeFolder()
{
    m_folderPath.clear();
    m_allFiles.clear();
    if (m_watcher->files().size() > 0 || m_watcher->directories().size() > 0) {
        m_watcher->removePaths(m_watcher->files());
        m_watcher->removePaths(m_watcher->directories());
    }
    emit folderClosed();
}

bool VaultManager::isOpen() const { return !m_folderPath.isEmpty(); }

QString VaultManager::folderPath() const { return m_folderPath; }
QString VaultManager::folderName() const { return QDir(m_folderPath).dirName(); }
QStringList VaultManager::allFiles() const { return m_allFiles; }

QStringList VaultManager::allNoteTitles() const
{
    QStringList titles;
    for (const QString& f : m_allFiles) {
        QFileInfo fi(f);
        QString base = fi.completeBaseName();
        if (fi.suffix() == "md" || fi.suffix() == "html") {
            titles.append(base);
        }
    }
    return titles;
}

QString VaultManager::readFile(const QString& relativePath) const
{
    return readFileContent(fullPath(relativePath));
}

bool VaultManager::writeFile(const QString& relativePath, const QString& content)
{
    QString path = fullPath(relativePath);
    QFile file(path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        stream << content;
        file.close();
        return true;
    }
    return false;
}

bool VaultManager::createFile(const QString& parentDir, const QString& name)
{
    QString fullParent = fullPath(parentDir);
    QString filePath = QDir(fullParent).absoluteFilePath(name);
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.close();
        QString rel = relativePath(filePath);
        m_allFiles.append(rel);
        m_watcher->addPath(filePath);
        emit fileCreated(rel);
        return true;
    }
    return false;
}

bool VaultManager::renameFile(const QString& relativePath, const QString& newName)
{
    QString oldFull = fullPath(relativePath);
    QFileInfo fi(oldFull);
    QString newFull = fi.dir().absoluteFilePath(newName);

    if (!QFile::rename(oldFull, newFull)) return false;

    QString newRel = this->relativePath(newFull);
    m_allFiles.removeOne(relativePath);
    m_allFiles.append(newRel);

    if (m_watcher->files().contains(oldFull)) {
        m_watcher->removePath(oldFull);
    }
    m_watcher->addPath(newFull);

    emit fileRenamed(relativePath, newRel);
    return true;
}

bool VaultManager::deleteFile(const QString& relativePath)
{
    QString full = fullPath(relativePath);
    if (!QFile::remove(full)) return false;

    m_allFiles.removeOne(relativePath);
    if (m_watcher->files().contains(full)) {
        m_watcher->removePath(full);
    }
    emit fileDeleted(relativePath);
    return true;
}

bool VaultManager::moveFile(const QString& relativePath, const QString& newParentDir)
{
    QString oldFull = fullPath(relativePath);
    QFileInfo fi(oldFull);
    QString newFull = QDir(fullPath(newParentDir)).absoluteFilePath(fi.fileName());

    if (!QFile::rename(oldFull, newFull)) return false;

    QString newRel = this->relativePath(newFull);
    m_allFiles.removeOne(relativePath);
    m_allFiles.append(newRel);

    if (m_watcher->files().contains(oldFull)) {
        m_watcher->removePath(oldFull);
    }
    m_watcher->addPath(newFull);

    emit fileMoved(relativePath, newRel);
    return true;
}

bool VaultManager::createDirectory(const QString& relativePath)
{
    QString full = fullPath(relativePath);
    if (QDir().mkpath(full)) {
        m_watcher->addPath(full);
        return true;
    }
    return false;
}

QString VaultManager::fullPath(const QString& relativePath) const
{
    if (QDir::isAbsolutePath(relativePath)) return relativePath;
    return QDir(m_folderPath).absoluteFilePath(relativePath);
}

QString VaultManager::relativePath(const QString& fullPath) const
{
    return QDir(m_folderPath).relativeFilePath(fullPath);
}

QString VaultManager::resolveWikilink(const QString& title) const
{
    static const QRegularExpression forbidden("[\\\\/:*?\"<>|]");
    QString sanitized = title;
    sanitized = sanitized.replace(forbidden, "").trimmed();

    for (const QString& f : m_allFiles) {
        QFileInfo fi(f);
        QString base = fi.completeBaseName();
        if (base.compare(sanitized, Qt::CaseInsensitive) == 0) {
            return f;
        }
    }

    QString mdCandidate = sanitized + ".md";
    QString htmlCandidate = sanitized + ".html";

    for (const QString& f : m_allFiles) {
        if (f.compare(mdCandidate, Qt::CaseInsensitive) == 0) return f;
        if (f.compare(htmlCandidate, Qt::CaseInsensitive) == 0) return f;
    }

    return QString();
}

void VaultManager::onDirectoryChanged(const QString& path)
{
    Q_UNUSED(path)
    scanFolder();
    addWatchPaths();
    emit folderChanged();
}

void VaultManager::onFileChanged(const QString& path)
{
    QString rel = relativePath(path);
    emit fileChanged(rel);
}

void VaultManager::scanFolder()
{
    m_allFiles.clear();
    QDirIterator it(m_folderPath, QStringList() << "*.md" << "*.html",
                    QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        it.next();
        m_allFiles.append(relativePath(it.filePath()));
    }
}

void VaultManager::addWatchPaths()
{
    QStringList currentFiles = m_watcher->files();
    QStringList currentDirs = m_watcher->directories();

    if (!currentFiles.isEmpty()) m_watcher->removePaths(currentFiles);
    if (!currentDirs.isEmpty()) m_watcher->removePaths(currentDirs);

    m_watcher->addPath(m_folderPath);

    QDirIterator it(m_folderPath, QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        it.next();
        m_watcher->addPath(it.filePath());
    }

    for (const QString& f : m_allFiles) {
        m_watcher->addPath(fullPath(f));
    }
}

QString VaultManager::readFileContent(const QString& path) const
{
    QFile file(path);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        return stream.readAll();
    }
    return QString();
}
