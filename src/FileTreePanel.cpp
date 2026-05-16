#include "FileTreePanel.h"
#include "FileTreeModel.h"
#include "FileFilterProxyModel.h"
#include "VaultManager.h"

#include <QHeaderView>
#include <QMenu>
#include <QInputDialog>
#include <QMessageBox>
#include <QFileInfo>
#include <QLabel>
#include <QDesktopServices>
#include <QUrl>
#include <QDir>

#ifdef Q_OS_WIN
#include <windows.h>
#include <shellapi.h>
#endif

FileTreePanel::FileTreePanel(VaultManager* folder, QWidget* parent)
    : QWidget(parent)
    , m_treeView(new QTreeView(this))
    , m_folder(folder)
    , m_model(nullptr)
    , m_proxyModel(new FileFilterProxyModel(this))
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
    connect(m_searchBox, &QLineEdit::textChanged,
            this, &FileTreePanel::onFilterChanged);
}

void FileTreePanel::setModel(FileTreeModel* model)
{
    m_model = model;
    m_proxyModel->setSourceModel(model);
    m_treeView->setModel(m_proxyModel);

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

    QModelIndex sourceIndex = m_model->findIndexByPath(relativePath);
    if (sourceIndex.isValid()) {
        QModelIndex proxyIndex = m_proxyModel->mapFromSource(sourceIndex);
        m_treeView->setCurrentIndex(proxyIndex);
        m_treeView->scrollTo(proxyIndex);
    }
}

void FileTreePanel::onFileClicked(const QModelIndex& proxyIndex)
{
    if (!m_model || !proxyIndex.isValid()) return;
    QModelIndex sourceIndex = m_proxyModel->mapToSource(proxyIndex);
    if (m_model->isDirectory(sourceIndex)) {
        m_treeView->setExpanded(proxyIndex, !m_treeView->isExpanded(proxyIndex));
        return;
    }
    QString path = m_model->filePath(sourceIndex);
    emit fileSelected(path);
}

void FileTreePanel::onFileDoubleClicked(const QModelIndex& proxyIndex)
{
    if (!m_model || !proxyIndex.isValid()) return;
    QModelIndex sourceIndex = m_proxyModel->mapToSource(proxyIndex);
    if (m_model->isDirectory(sourceIndex)) return;
    QString path = m_model->filePath(sourceIndex);
    emit fileSelected(path);
}

void FileTreePanel::onNewFile()
{
    QModelIndex proxyCurrent = m_treeView->currentIndex();
    QModelIndex current = m_proxyModel->mapToSource(proxyCurrent);
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
    QModelIndex proxyCurrent = m_treeView->currentIndex();
    QModelIndex current = m_proxyModel->mapToSource(proxyCurrent);
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
    QModelIndex proxyCurrent = m_treeView->currentIndex();
    QModelIndex current = m_proxyModel->mapToSource(proxyCurrent);
    if (!current.isValid() || !m_model) return;

    QString path = m_model->filePath(current);
    bool isDir = m_model->isDirectory(current);

    QString msg = isDir ? tr("Move folder '%1' to trash?").arg(path)
                        : tr("Move file '%1' to trash?").arg(path);

    if (QMessageBox::question(this, tr("Confirm"), msg,
                               QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        if (m_folder->deleteFile(path)) {
            refresh();
        }
    }
}

void FileTreePanel::onRename()
{
    QModelIndex proxyCurrent = m_treeView->currentIndex();
    if (!proxyCurrent.isValid() || !m_model) return;

    m_treeView->edit(proxyCurrent);
}

void FileTreePanel::onFilterChanged()
{
    m_proxyModel->setFilterFixedString(m_searchBox->text());
}

void FileTreePanel::onOpenInExplorer()
{
    QModelIndex proxyIndex = m_treeView->currentIndex();
    if (!proxyIndex.isValid() || !m_model) return;
    QModelIndex sourceIndex = m_proxyModel->mapToSource(proxyIndex);
    QString relPath = m_model->filePath(sourceIndex);
    QString absPath = m_folder->fullPath(relPath);

#ifdef Q_OS_WIN
    QString nativePath = QDir::toNativeSeparators(absPath);
    if (m_model->isDirectory(sourceIndex)) {
        ShellExecuteW(nullptr, L"open",
                      nativePath.toStdWString().c_str(),
                      nullptr, nullptr, SW_SHOWNORMAL);
    } else {
        QString param = QStringLiteral("/select,\"%1\"").arg(nativePath);
        ShellExecuteW(nullptr, L"open", L"explorer",
                      param.toStdWString().c_str(),
                      nullptr, SW_SHOWNORMAL);
    }
#else
    QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(absPath).path()));
#endif
}

void FileTreePanel::onOpenInBrowser()
{
    QModelIndex proxyIndex = m_treeView->currentIndex();
    if (!proxyIndex.isValid() || !m_model) return;
    QModelIndex sourceIndex = m_proxyModel->mapToSource(proxyIndex);
    QString relPath = m_model->filePath(sourceIndex);
    QString absPath = m_folder->fullPath(relPath);
    QDesktopServices::openUrl(QUrl::fromLocalFile(absPath));
}

void FileTreePanel::showContextMenu(const QPoint& pos)
{
    QModelIndex proxyIndex = m_treeView->indexAt(pos);
    if (proxyIndex.isValid()) {
        m_treeView->setCurrentIndex(proxyIndex);
    }
    QModelIndex sourceIndex = m_proxyModel->mapToSource(proxyIndex);

    QMenu menu;

    QAction* newFileAction = menu.addAction(tr("New File"));
    menu.addSeparator();
    QAction* newFolderAction = menu.addAction(tr("New Folder"));

    QAction* renameAction = nullptr;
    QAction* deleteAction = nullptr;
    QAction* openExplorerAction = nullptr;
    QAction* openBrowserAction = nullptr;

    if (proxyIndex.isValid() && m_model) {
        menu.addSeparator();
        openExplorerAction = menu.addAction(tr("Open in Explorer"));
        QString relPath = m_model->filePath(sourceIndex);
        if (relPath.endsWith(".html", Qt::CaseInsensitive)) {
            openBrowserAction = menu.addAction(tr("Open in Browser"));
        }
        menu.addSeparator();
        renameAction = menu.addAction(tr("Rename"));
        deleteAction = menu.addAction(tr("Move to Trash"));
    }

    QAction* selected = menu.exec(m_treeView->viewport()->mapToGlobal(pos));
    if (!selected) return;

    if (selected == newFileAction) {
        onNewFile();
    } else if (selected == newFolderAction) {
        onNewFolder();
    } else if (selected == openExplorerAction) {
        onOpenInExplorer();
    } else if (selected == openBrowserAction) {
        onOpenInBrowser();
    } else if (selected == renameAction) {
        onRename();
    } else if (selected == deleteAction) {
        onDelete();
    }
}
