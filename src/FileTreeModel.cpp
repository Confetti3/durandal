#include "FileTreeModel.h"
#include "VaultManager.h"
#include <QDir>
#include <QFileInfo>
#include <QMimeData>
#include <QIcon>
#include <QUrl>
#include <QRegularExpression>
#include <QDataStream>
#include <QSvgRenderer>
#include <QPainter>
#include <algorithm>

static const char* SVG_DOC_TEMPLATE = R"(<svg width="16" height="16" xmlns="http://www.w3.org/2000/svg">
  <rect x="1.5" y="1.5" width="10" height="13" rx="1.5" fill="%1" stroke="%2" stroke-width="1"/>
  <polygon points="9.5,1.5 11.5,1.5 11.5,4.5" fill="%3"/>
  <line x1="4" y1="7" x2="10" y2="7" stroke="%4" stroke-width="1" stroke-linecap="round"/>
  <line x1="4" y1="10" x2="10" y2="10" stroke="%4" stroke-width="1" stroke-linecap="round"/>
  <line x1="4" y1="13" x2="8" y2="13" stroke="%4" stroke-width="1" stroke-linecap="round"/>
</svg>)";

QIcon FileTreeModel::svgDocIcon(const QString& fillColor, const QString& strokeColor, const QString& cornerColor)
{
    QString svg = QString(SVG_DOC_TEMPLATE)
                      .arg(fillColor, strokeColor, cornerColor, "#ffffff");
    QByteArray data = svg.toUtf8();

    QSvgRenderer renderer(data);
    QPixmap pixmap(16, 16);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    renderer.render(&p);
    p.end();
    return QIcon(pixmap);
}

FileTreeModel::FileTreeModel(VaultManager* folder, QObject* parent)
    : QAbstractItemModel(parent)
    , m_folder(folder)
    , m_rootNode(new TreeNode)
{
    m_rootNode->isDir = true;
    m_folderIcon = m_iconProvider.icon(QFileIconProvider::Folder);
    m_mdIcon   = svgDocIcon("#7BA7D9", "#5A8ABF", "#4A7BB5");   // soft muted blue
    m_htmlIcon = svgDocIcon("#E8E4DA", "#8A95A5", "#7A8696");   // warm white, cool gray
    m_fileIcon = m_iconProvider.icon(QFileIconProvider::File);
}

FileTreeModel::~FileTreeModel()
{
    delete m_rootNode;
}

void FileTreeModel::setRootPath(const QString& path)
{
    beginResetModel();
    clear();
    m_rootNode->relativePath = path;
    m_rootNode->name = QDir(path).dirName();
    m_rootNode->isDir = true;
    populateChildren(m_rootNode);
    endResetModel();
}

void FileTreeModel::refresh()
{
    if (!m_folder->isOpen()) return;
    beginResetModel();
    clear();
    m_rootNode->isDir = true;
    m_rootNode->name = QDir(m_folder->folderPath()).dirName();
    m_rootNode->relativePath = "";
    m_rootNode->populated = false;
    populateChildren(m_rootNode);
    endResetModel();
}

QString FileTreeModel::filePath(const QModelIndex& index) const
{
    TreeNode* node = nodeFromIndex(index);
    if (!node) return QString();
    return node->relativePath;
}

bool FileTreeModel::isDirectory(const QModelIndex& index) const
{
    TreeNode* node = nodeFromIndex(index);
    return node && node->isDir;
}

QModelIndex FileTreeModel::findIndexByPath(const QString& path, const QModelIndex& parent) const
{
    TreeNode* parentNode = parent.isValid() ? nodeFromIndex(parent) : m_rootNode;
    if (!parentNode) return QModelIndex();

    if (!parentNode->populated) {
        const_cast<FileTreeModel*>(this)->populateChildren(parentNode);
    }

    for (int i = 0; i < parentNode->children.size(); ++i) {
        if (parentNode->children[i]->relativePath == path) {
            return createIndex(i, 0, parentNode->children[i]);
        }
    }
    return QModelIndex();
}

QModelIndex FileTreeModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent)) return QModelIndex();

    TreeNode* parentNode = parent.isValid() ? nodeFromIndex(parent) : m_rootNode;
    if (!parentNode) return QModelIndex();

    if (!parentNode->populated) {
        const_cast<FileTreeModel*>(this)->populateChildren(parentNode);
    }

    if (row < parentNode->children.size()) {
        return createIndex(row, column, parentNode->children.at(row));
    }
    return QModelIndex();
}

QModelIndex FileTreeModel::parent(const QModelIndex& index) const
{
    if (!index.isValid()) return QModelIndex();

    TreeNode* childNode = nodeFromIndex(index);
    TreeNode* parentNode = childNode ? childNode->parent : nullptr;

    if (!parentNode || parentNode == m_rootNode) return QModelIndex();
    return indexFromNode(parentNode);
}

int FileTreeModel::rowCount(const QModelIndex& parent) const
{
    TreeNode* parentNode = parent.isValid() ? nodeFromIndex(parent) : m_rootNode;
    if (!parentNode || !parentNode->isDir) return 0;

    if (!parentNode->populated) {
        const_cast<FileTreeModel*>(this)->populateChildren(parentNode);
    }

    return parentNode->children.size();
}

int FileTreeModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return 1;
}

QVariant FileTreeModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) return QVariant();

    TreeNode* node = nodeFromIndex(index);
    if (!node) return QVariant();

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        return node->name;
    }

    if (role == Qt::DecorationRole) {
        if (node->isDir) return m_folderIcon;
        QString path = node->relativePath;
        if (path.endsWith(".md", Qt::CaseInsensitive)) {
            return m_mdIcon;
        }
        if (path.endsWith(".html", Qt::CaseInsensitive)) {
            return m_htmlIcon;
        }
        return m_fileIcon;
    }

    if (role == Qt::ToolTipRole) {
        return node->relativePath;
    }

    return QVariant();
}

bool FileTreeModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || role != Qt::EditRole) return false;

    TreeNode* node = nodeFromIndex(index);
    if (!node || node == m_rootNode) return false;

    QString newName = value.toString().trimmed();
    if (newName.isEmpty() || newName == node->name) return false;

    static const QRegularExpression forbidden(R"([\\/:*?"<>|])");
    if (newName.contains(forbidden)) return false;

    if (m_folder->renameFile(node->relativePath, newName)) {
        node->name = newName;
        node->relativePath = m_folder->relativePath(
            QFileInfo(m_folder->fullPath(node->relativePath)).dir().absoluteFilePath(newName));
        emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
        return true;
    }
    return false;
}

Qt::ItemFlags FileTreeModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags defaultFlags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    if (!index.isValid()) {
        return defaultFlags | Qt::ItemIsDropEnabled;
    }

    TreeNode* node = nodeFromIndex(index);
    if (!node) return defaultFlags;

    Qt::ItemFlags flags = defaultFlags;
    flags |= Qt::ItemIsDragEnabled;

    if (node->isDir) {
        flags |= Qt::ItemIsDropEnabled;
    }

    if (node != m_rootNode) {
        flags |= Qt::ItemIsEditable;
    }

    return flags;
}

QVariant FileTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(section)
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return tr("Files");
    }
    return QVariant();
}

QStringList FileTreeModel::mimeTypes() const
{
    return {QStringLiteral("application/x-obsidianqt-filelist")};
}

QMimeData* FileTreeModel::mimeData(const QModelIndexList& indexes) const
{
    QMimeData* mimeData = new QMimeData();
    QByteArray encodedData;
    QDataStream stream(&encodedData, QIODevice::WriteOnly);

    for (const QModelIndex& index : indexes) {
        if (index.isValid()) {
            QString path = filePath(index);
            stream << path;
        }
    }

    mimeData->setData(QStringLiteral("application/x-obsidianqt-filelist"), encodedData);
    return mimeData;
}

bool FileTreeModel::dropMimeData(const QMimeData* data, Qt::DropAction action,
                                  int row, int column, const QModelIndex& parent)
{
    if (!canDropMimeData(data, action, row, column, parent)) return false;

    QByteArray encodedData = data->data(QStringLiteral("application/x-obsidianqt-filelist"));
    QDataStream stream(&encodedData, QIODevice::ReadOnly);
    QStringList paths;

    while (!stream.atEnd()) {
        QString path;
        stream >> path;
        if (!path.isEmpty()) paths.append(path);
    }

    if (paths.isEmpty()) return false;

    TreeNode* parentNode = parent.isValid() ? nodeFromIndex(parent) : m_rootNode;
    if (!parentNode) return false;

    QString targetDir = parentNode->relativePath.isEmpty() ? "" : parentNode->relativePath;

    for (const QString& sourcePath : paths) {
        if (m_folder->moveFile(sourcePath, targetDir)) {
            QString newPath = QDir(targetDir).filePath(QFileInfo(sourcePath).fileName());
            if (newPath.startsWith("/")) newPath = newPath.mid(1);
            emit fileMoved(sourcePath, newPath);
        }
    }

    refresh();
    return true;
}

Qt::DropActions FileTreeModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

bool FileTreeModel::removeRows(int row, int count, const QModelIndex& parent)
{
    TreeNode* parentNode = parent.isValid() ? nodeFromIndex(parent) : m_rootNode;
    if (!parentNode) return false;

    if (!parentNode->populated) populateChildren(parentNode);

    if (row < 0 || row + count > parentNode->children.size()) return false;

    beginRemoveRows(parent, row, row + count - 1);

    for (int i = row + count - 1; i >= row; --i) {
        TreeNode* child = parentNode->children.at(i);
        if (!child->isDir) {
            m_folder->deleteFile(child->relativePath);
        }
        parentNode->children.removeAt(i);
        delete child;
    }

    endRemoveRows();
    return true;
}

FileTreeModel::TreeNode* FileTreeModel::nodeFromIndex(const QModelIndex& index) const
{
    if (index.isValid()) {
        return static_cast<TreeNode*>(index.internalPointer());
    }
    return nullptr;
}

QModelIndex FileTreeModel::indexFromNode(TreeNode* node) const
{
    if (!node || !node->parent) return QModelIndex();

    TreeNode* parent = node->parent;
    int row = parent->children.indexOf(node);
    if (row >= 0) {
        return createIndex(row, 0, node);
    }
    return QModelIndex();
}

void FileTreeModel::populateChildren(TreeNode* node)
{
    if (!node || node->populated) return;

    QString dirPath = m_folder->fullPath(node->relativePath);
    node->populated = true;

    QDir dir(dirPath);
    if (!dir.exists()) return;

    auto entryList = dir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot, QDir::DirsFirst | QDir::Name);

    for (const QFileInfo& fi : entryList) {
        TreeNode* child = new TreeNode;
        child->name = fi.fileName();
        child->isDir = fi.isDir();
        child->parent = node;

        if (node == m_rootNode) {
            child->relativePath = fi.fileName();
        } else {
            QString prefix = node->relativePath;
            child->relativePath = prefix + "/" + fi.fileName();
        }

        if (!child->isDir) {
            QString suffix = fi.suffix().toLower();
            if (suffix != "md" && suffix != "html") {
                delete child;
                continue;
            }
        }

        node->children.append(child);
    }
}

void FileTreeModel::clear()
{
    qDeleteAll(m_rootNode->children);
    m_rootNode->children.clear();
    m_rootNode->populated = false;
}

bool FileTreeModel::canDropMimeData(const QMimeData* data, Qt::DropAction action,
                                     int row, int column, const QModelIndex& parent) const
{
    Q_UNUSED(row)
    Q_UNUSED(column)

    if (action != Qt::MoveAction) return false;
    if (!data->hasFormat(QStringLiteral("application/x-obsidianqt-filelist"))) return false;

    if (parent.isValid()) {
        TreeNode* node = nodeFromIndex(parent);
        return node && node->isDir;
    }
    return true;
}
