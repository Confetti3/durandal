#ifndef BACKLINKSPANEL_H
#define BACKLINKSPANEL_H

#include <QWidget>
#include <QListWidget>
#include <QVBoxLayout>
#include <QLabel>

class VaultManager;
class WikilinkParser;

class BacklinksPanel : public QWidget
{
    Q_OBJECT

public:
    explicit BacklinksPanel(VaultManager* folder, QWidget* parent = nullptr);

    void updateBacklinks(const QString& notePath);
    void clear();

signals:
    void backlinkSelected(const QString& filePath);

private slots:
    void onItemClicked(QListWidgetItem* item);

private:
    VaultManager* m_folder;
    WikilinkParser* m_parser;
    QListWidget* m_listWidget;
    QLabel* m_headerLabel;

    void setupUI();
};

#endif // BACKLINKSPANEL_H
