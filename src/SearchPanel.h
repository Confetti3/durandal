#ifndef SEARCHPANEL_H
#define SEARCHPANEL_H

#include <QWidget>
#include <QLineEdit>
#include <QListWidget>
#include <QVBoxLayout>
#include <QLabel>

class SearchEngine;

class SearchPanel : public QWidget
{
    Q_OBJECT

public:
    explicit SearchPanel(SearchEngine* engine, QWidget* parent = nullptr);

    void refreshIndex();

signals:
    void resultSelected(const QString& filePath);

private slots:
    void onSearchTextChanged(const QString& text);
    void onResultClicked(QListWidgetItem* item);

private:
    SearchEngine* m_engine;
    QLineEdit* m_searchBox;
    QListWidget* m_listWidget;
    QLabel* m_headerLabel;

    void setupUI();
    void performSearch(const QString& query);
};

#endif // SEARCHPANEL_H
