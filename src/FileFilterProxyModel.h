#ifndef FILEFILTERPROXYMODEL_H
#define FILEFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

class FileFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit FileFilterProxyModel(QObject* parent = nullptr);

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

private:
    bool nameMatches(const QModelIndex& source_index) const;
    bool hasMatchingDescendant(const QModelIndex& source_parent) const;
    bool hasMatchingAncestor(const QModelIndex& source_parent) const;
};

#endif // FILEFILTERPROXYMODEL_H
