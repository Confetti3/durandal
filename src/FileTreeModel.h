#ifndef FILETREEMODEL_H
#define FILETREEMODEL_H

#include <QAbstractItemModel>
#include <QIcon>
#include <QFileIconProvider>

class VaultManager;

class FileTreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit FileTreeModel(VaultManager* folder, QObject* parent = nullptr);
    ~FileTreeModel();

    void setRootPath(const QString& path);
    void refresh();
    QString filePath(const QModelIndex& index) const;
    bool isDirectory(const QModelIndex& index) const;
    QModelIndex findIndexByPath(const QString& path, const QModelIndex& parent = QModelIndex()) const;

    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    QStringList mimeTypes() const override;
    QMimeData* mimeData(const QModelIndexList& indexes) const override;
    bool dropMimeData(const QMimeData* data, Qt::DropAction action,
                      int row, int column, const QModelIndex& parent) override;
    Qt::DropActions supportedDropActions() const override;

    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;

signals:
    void fileMoved(const QString& oldPath, const QString& newPath);

private:
    struct TreeNode {
        QString name;
        QString relativePath;
        bool isDir;
        TreeNode* parent;
        QList<TreeNode*> children;
        bool populated;

        TreeNode() : isDir(false), parent(nullptr), populated(false) {}
        ~TreeNode() { qDeleteAll(children); }
    };

    VaultManager* m_folder;
    TreeNode* m_rootNode;
    QIcon m_folderIcon;
    QIcon m_mdIcon;
    QIcon m_htmlIcon;
    QIcon m_fileIcon;
    QFileIconProvider m_iconProvider;

    static QIcon svgDocIcon(const QString& fillColor, const QString& strokeColor, const QString& cornerColor);

    TreeNode* nodeFromIndex(const QModelIndex& index) const;
    QModelIndex indexFromNode(TreeNode* node) const;
    void populateChildren(TreeNode* node);
    void clear();
    bool canDropMimeData(const QMimeData* data, Qt::DropAction action,
                         int row, int column, const QModelIndex& parent) const;
};

#endif // FILETREEMODEL_H
