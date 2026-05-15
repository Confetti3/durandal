#include "FileFilterProxyModel.h"
#include "FileTreeModel.h"

FileFilterProxyModel::FileFilterProxyModel(QObject* parent)
    : QSortFilterProxyModel(parent)
{
    setFilterCaseSensitivity(Qt::CaseInsensitive);
}

bool FileFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    QModelIndex source_index = sourceModel()->index(source_row, 0, source_parent);
    if (!source_index.isValid())
        return false;

    // Empty filter accepts everything
    if (filterRegularExpression().pattern().isEmpty())
        return true;

    // Check if this row's name matches the filter
    if (nameMatches(source_index))
        return true;

    // For directories: accept if any descendant matches
    FileTreeModel* fileModel = qobject_cast<FileTreeModel*>(sourceModel());
    if (fileModel && fileModel->isDirectory(source_index)) {
        if (hasMatchingDescendant(source_index))
            return true;
    }

    // Accept if any ancestor matches (show children of matching directories)
    if (hasMatchingAncestor(source_parent))
        return true;

    return false;
}

bool FileFilterProxyModel::nameMatches(const QModelIndex& source_index) const
{
    QString text = sourceModel()->data(source_index, filterRole()).toString();
    return filterRegularExpression().match(text).hasMatch();
}

bool FileFilterProxyModel::hasMatchingDescendant(const QModelIndex& source_parent) const
{
    int rows = sourceModel()->rowCount(source_parent);
    for (int i = 0; i < rows; ++i) {
        QModelIndex child = sourceModel()->index(i, 0, source_parent);
        if (!child.isValid())
            continue;

        if (nameMatches(child))
            return true;

        FileTreeModel* fileModel = qobject_cast<FileTreeModel*>(sourceModel());
        if (fileModel && fileModel->isDirectory(child)) {
            if (hasMatchingDescendant(child))
                return true;
        }
    }
    return false;
}

bool FileFilterProxyModel::hasMatchingAncestor(const QModelIndex& source_parent) const
{
    if (!source_parent.isValid())
        return false;

    if (nameMatches(source_parent))
        return true;

    return hasMatchingAncestor(source_parent.parent());
}
