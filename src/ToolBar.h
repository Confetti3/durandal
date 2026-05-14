#ifndef TOOLBAR_H
#define TOOLBAR_H

#include <QToolBar>
#include <QAction>
#include <QList>

class ToolBar : public QToolBar
{
    Q_OBJECT

public:
    explicit ToolBar(QWidget* parent = nullptr);
    void setDark(bool dark);

signals:
    void boldClicked();
    void italicClicked();
    void heading1Clicked();
    void heading2Clicked();
    void heading3Clicked();
    void bulletListClicked();
    void numberedListClicked();
    void codeBlockClicked();
    void linkClicked();
    void wikilinkClicked();

private:
    QList<QAction*> m_actions;
    void setupActions(bool dark);
    static QIcon makeTextIcon(const QString& text, const QColor& color, bool bold = false, bool italic = false);
    static QIcon makeHeadingIcon(int level, const QColor& color);
    static QIcon makeListIcon(bool numbered, const QColor& color);
    static QIcon makeLinkIcon(const QColor& color);
};

#endif // TOOLBAR_H
