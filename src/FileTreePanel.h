#ifndef FILETREEPANEL_H
#define FILETREEPANEL_H

#include <QWidget>
#include <QTreeView>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

class FileTreeModel;
class FileFilterProxyModel;
class VaultManager;

class FileTreePanel : public QWidget
{
    Q_OBJECT

public:
    explicit FileTreePanel(VaultManager* folder, QWidget* parent = nullptr);

    void setModel(FileTreeModel* model);
    void refresh();
    void selectFile(const QString& relativePath);

signals:
    void fileSelected(const QString& relativePath);

public slots:
    void onNewFile();
    void onNewFolder();

private slots:
    void onFileClicked(const QModelIndex& proxyIndex);
    void onFileDoubleClicked(const QModelIndex& proxyIndex);
    void onDelete();
    void onRename();
    void onFilterChanged();
    void onOpenInExplorer();
    void onOpenInBrowser();
    void showContextMenu(const QPoint& pos);

private:
    QTreeView* m_treeView;
    VaultManager* m_folder;
    FileTreeModel* m_model;
    FileFilterProxyModel* m_proxyModel;
    QLineEdit* m_searchBox;

    void setupUI();
};

#endif // FILETREEPANEL_H
