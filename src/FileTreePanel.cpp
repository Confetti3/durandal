#include "FileTreePanel.h"
#include "FileTreeModel.h"
#include "VaultManager.h"

#include <QHeaderView>
#include <QMenu>
#include <QInputDialog>
#include <QMessageBox>
#include <QSortFilterProxyModel>
#include <QFileInfo>
#include <QLabel>

FileTreePanel::FileTreePanel(VaultManager* folder, QWidget* parent)
    : QWidget(parent)
    , m_treeView(new QTreeView(this))
    , m_folder(folder)
    , m_model(nullptr)
    , m_searchBox(new QLineEdit(this))
{
    setupUI();
}

void FileTreePanel::setupUI()
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);

    m_searchBox->setPlaceholderText(tr("Filter files..."));
    m_searchBox->setClearButtonEnabled(true);
    layout->addWidget(m_searchBox);

    m_treeView->setHeaderHidden(true);
    m_treeView->setAnimated(true);
    m_treeView->setIndentation(16);
    m_treeView->setDragEnabled(true);
    m_treeView->setAcceptDrops(true);
    m_treeView->setDropIndicatorShown(true);
    m_treeView->setDragDropMode(QAbstractItemView::InternalMove);
    m_treeView->setDefaultDropAction(Qt::MoveAction);
    m_treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_treeView->setEditTriggers(QAbstractItemView::SelectedClicked |
                                 QAbstractItemView::EditKeyPressed);
    layout->addWidget(m_treeView);

    connect(m_treeView, &QTreeView::clicked,
            this, &FileTreePanel::onFileClicked);
    connect(m_treeView, &QTreeView::doubleClicked,
            this, &FileTreePanel::onFileDoubleClicked);
    connect(m_treeView, &QTreeView::customContextMenuRequested,
            this, &FileTreePanel::showContextMenu);
}

void FileTreePanel::setModel(FileTreeModel* model)
{
    m_model = model;
    m_treeView->setModel(model);

    if (m_treeView->header()) {
        m_treeView->header()->setStretchLastSection(true);
    }
}

void FileTreePanel::refresh()
{
    if (m_model) m_model->refresh();
    m_treeView->expandToDepth(1);
}

void FileTreePanel::selectFile(const QString& relativePath)
{
    if (!m_model) return;

    QModelIndex index = m_model->findIndexByPath(relativePath);
    if (index.isValid()) {
        m_treeView->setCurrentIndex(index);
        m_treeView->scrollTo(index);
    }
}

void FileTreePanel::onFileClicked(const QModelIndex& index)
{
    if (!m_model || !index.isValid()) return;
    if (m_model->isDirectory(index)) {
        m_treeView->setExpanded(index, !m_treeView->isExpanded(index));
        return;
    }
    QString path = m_model->filePath(index);
    emit fileSelected(path);
}

void FileTreePanel::onFileDoubleClicked(const QModelIndex& index)
{
    if (!m_model || !index.isValid() || m_model->isDirectory(index)) return;
    QString path = m_model->filePath(index);
    emit fileSelected(path);
}

void FileTreePanel::onNewFile()
{
    QModelIndex current = m_treeView->currentIndex();
    QString parentDir;

    if (current.isValid() && m_model) {
        parentDir = m_model->isDirectory(current)
                        ? m_model->filePath(current)
                        : QFileInfo(m_model->filePath(current)).path();
    }

    bool ok;
    QString name = QInputDialog::getText(this, tr("New File"),
                                         tr("File name (with extension):"),
                                         QLineEdit::Normal, "new-note.md", &ok);
    if (ok && !name.isEmpty()) {
        if (!name.endsWith(".md") && !name.endsWith(".html")) {
            name += ".md";
        }
        if (m_folder->createFile(parentDir, name)) {
            refresh();
        }
    }
}

void FileTreePanel::onNewFolder()
{
    QModelIndex current = m_treeView->currentIndex();
    QString parentDir;

    if (current.isValid() && m_model) {
        parentDir = m_model->isDirectory(current)
                        ? m_model->filePath(current)
                        : QFileInfo(m_model->filePath(current)).path();
    }

    bool ok;
    QString name = QInputDialog::getText(this, tr("New Folder"),
                                         tr("Folder name:"),
                                         QLineEdit::Normal, "New Folder", &ok);
    if (ok && !name.isEmpty()) {
        QString fullPath = parentDir.isEmpty() ? name : parentDir + "/" + name;
        if (m_folder->createDirectory(fullPath)) {
            refresh();
        }
    }
}

void FileTreePanel::onDelete()
{
    QModelIndex current = m_treeView->currentIndex();
    if (!current.isValid() || !m_model) return;

    QString path = m_model->filePath(current);
    bool isDir = m_model->isDirectory(current);

    QString msg = isDir ? tr("Delete folder '%1' and all its contents?").arg(path)
                        : tr("Delete file '%1'?").arg(path);

    if (QMessageBox::question(this, tr("Confirm Delete"), msg,
                               QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        if (m_folder->deleteFile(path)) {
            refresh();
        }
    }
}

void FileTreePanel::onRename()
{
    QModelIndex current = m_treeView->currentIndex();
    if (!current.isValid() || !m_model) return;

    m_treeView->edit(current);
}

void FileTreePanel::showContextMenu(const QPoint& pos)
{
    QModelIndex index = m_treeView->indexAt(pos);

    QMenu menu;

    QAction* newFileAction = menu.addAction(tr("New File"));
    menu.addSeparator();
    QAction* newFolderAction = menu.addAction(tr("New Folder"));

    QAction* renameAction = nullptr;
    QAction* deleteAction = nullptr;

    if (index.isValid() && m_model) {
        menu.addSeparator();
        renameAction = menu.addAction(tr("Rename"));
        deleteAction = menu.addAction(tr("Delete"));
    }

    QAction* selected = menu.exec(m_treeView->viewport()->mapToGlobal(pos));
    if (!selected) return;

    if (selected == newFileAction) {
        onNewFile();
    } else if (selected == newFolderAction) {
        onNewFolder();
    } else if (selected == renameAction) {
        onRename();
    } else if (selected == deleteAction) {
        onDelete();
    }
}
