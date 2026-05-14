#ifndef TOOLBAR_H
#define TOOLBAR_H

#include <QToolBar>
#include <QAction>
#include <QList>
#include <QComboBox>

class ToolBar : public QToolBar
{
    Q_OBJECT

public:
    explicit ToolBar(QWidget* parent = nullptr);
    void setDark(bool dark);
    void setFontSize(int size);

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
    void fontSizeChanged(int size);

private:
    QList<QAction*> m_actions;
    QComboBox* m_fontSizeCombo;
    void setupActions(bool dark);
    static QIcon makeTextIcon(const QString& text, const QColor& color, bool bold = false, bool italic = false);
    static QIcon makeHeadingIcon(int level, const QColor& color);
    static QIcon makeListIcon(bool numbered, const QColor& color);
    static QIcon makeLinkIcon(const QColor& color);
};

#endif // TOOLBAR_H
