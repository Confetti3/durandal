#ifndef TAGPANEL_H
#define TAGPANEL_H

#include <QWidget>
#include <QListWidget>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QLabel>

class VaultManager;
class TagParser;

class TagPanel : public QWidget
{
    Q_OBJECT

public:
    explicit TagPanel(VaultManager* folder, QWidget* parent = nullptr);

    void refresh();
    void clear();

signals:
    void tagSelected(const QString& tag);
    void fileSelectedByTag(const QString& filePath);

private slots:
    void onItemClicked(QListWidgetItem* item);
    void onFilterChanged(const QString& text);

private:
    VaultManager* m_folder;
    TagParser* m_parser;
    QListWidget* m_listWidget;
    QLineEdit* m_filterBox;
    QLabel* m_headerLabel;
    QMap<QString, QStringList> m_tagIndex;

    void setupUI();
    void buildTagIndex();
    void populateList(const QString& filter = QString());
};

#endif // TAGPANEL_H
