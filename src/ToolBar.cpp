#include "ToolBar.h"
#include <QPainter>
#include <QComboBox>

QIcon ToolBar::makeTextIcon(const QString& text, const QColor& color, bool bold, bool italic)
{
    QPixmap px(20, 20);
    px.fill(Qt::transparent);
    QPainter p(&px);
    p.setRenderHint(QPainter::Antialiasing);
    QFont f = p.font();
    f.setPointSize(10);
    if (bold) f.setBold(true);
    if (italic) f.setItalic(true);
    p.setFont(f);
    p.setPen(color);
    p.drawText(px.rect(), Qt::AlignCenter, text);
    return QIcon(px);
}

QIcon ToolBar::makeHeadingIcon(int level, const QColor& color)
{
    QPixmap px(20, 20);
    px.fill(Qt::transparent);
    QPainter p(&px);
    p.setRenderHint(QPainter::Antialiasing);
    QFont f = p.font();
    f.setPointSize(9);
    f.setBold(true);
    p.setFont(f);
    p.setPen(color);
    p.drawText(px.rect().adjusted(0, -3, 0, -3), Qt::AlignCenter, "H");
    p.setPen(QPen(color, 1.5));
    int y = 15;
    for (int i = 0; i < level; ++i) {
        p.drawLine(4, y + i * 3, 16, y + i * 3);
    }
    return QIcon(px);
}

QIcon ToolBar::makeListIcon(bool numbered, const QColor& color)
{
    QPixmap px(20, 20);
    px.fill(Qt::transparent);
    QPainter p(&px);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(color);
    if (numbered) {
        QFont f = p.font();
        f.setPointSize(8);
        f.setBold(true);
        p.setFont(f);
        p.drawText(px.rect(), Qt::AlignCenter, "1.");
    } else {
        for (int i = 0; i < 3; ++i) {
            p.drawEllipse(QPointF(4, 6 + i * 5), 1.5, 1.5);
            p.drawLine(8, 6 + i * 5, 16, 6 + i * 5);
        }
    }
    return QIcon(px);
}

QIcon ToolBar::makeLinkIcon(const QColor& color)
{
    QPixmap px(20, 20);
    px.fill(Qt::transparent);
    QPainter p(&px);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(color, 1.8));
    p.setBrush(Qt::NoBrush);
    p.drawRoundedRect(3, 7, 7, 6, 2, 2);
    p.drawRoundedRect(10, 7, 7, 6, 2, 2);
    return QIcon(px);
}

ToolBar::ToolBar(QWidget* parent)
    : QToolBar(tr("Formatting"), parent)
{
    setMovable(false);
    setIconSize(QSize(20, 20));
    setupActions(false);
}

void ToolBar::setDark(bool dark)
{
    // Remove existing actions and recreate with correct colors
    for (QAction* a : m_actions) {
        removeAction(a);
        delete a;
    }
    m_actions.clear();
    setupActions(dark);
}

void ToolBar::setupActions(bool dark)
{
    QColor iconColor = dark ? QColor("#cccccc") : QColor("#2a2a2a");

    QAction* boldAction = addAction(makeTextIcon("B", iconColor, true), tr("Bold"));
    boldAction->setToolTip(tr("Bold (Ctrl+B)"));
    boldAction->setShortcut(QKeySequence::Bold);
    connect(boldAction, &QAction::triggered, this, &ToolBar::boldClicked);
    m_actions.append(boldAction);

    QAction* italicAction = addAction(makeTextIcon("I", iconColor, false, true), tr("Italic"));
    italicAction->setToolTip(tr("Italic (Ctrl+I)"));
    italicAction->setShortcut(QKeySequence::Italic);
    connect(italicAction, &QAction::triggered, this, &ToolBar::italicClicked);
    m_actions.append(italicAction);

    addSeparator();

    QAction* h1Action = addAction(makeHeadingIcon(1, iconColor), tr("Heading 1"));
    h1Action->setToolTip(tr("Heading 1"));
    connect(h1Action, &QAction::triggered, this, &ToolBar::heading1Clicked);
    m_actions.append(h1Action);

    QAction* h2Action = addAction(makeHeadingIcon(2, iconColor), tr("Heading 2"));
    h2Action->setToolTip(tr("Heading 2"));
    connect(h2Action, &QAction::triggered, this, &ToolBar::heading2Clicked);
    m_actions.append(h2Action);

    QAction* h3Action = addAction(makeHeadingIcon(3, iconColor), tr("Heading 3"));
    h3Action->setToolTip(tr("Heading 3"));
    connect(h3Action, &QAction::triggered, this, &ToolBar::heading3Clicked);
    m_actions.append(h3Action);

    addSeparator();

    QAction* ulAction = addAction(makeListIcon(false, iconColor), tr("Bullet List"));
    ulAction->setToolTip(tr("Bullet List"));
    connect(ulAction, &QAction::triggered, this, &ToolBar::bulletListClicked);
    m_actions.append(ulAction);

    QAction* olAction = addAction(makeListIcon(true, iconColor), tr("Numbered List"));
    olAction->setToolTip(tr("Numbered List"));
    connect(olAction, &QAction::triggered, this, &ToolBar::numberedListClicked);
    m_actions.append(olAction);

    addSeparator();

    QAction* codeAction = addAction(makeTextIcon("</>", iconColor), tr("Code Block"));
    codeAction->setToolTip(tr("Code Block"));
    connect(codeAction, &QAction::triggered, this, &ToolBar::codeBlockClicked);
    m_actions.append(codeAction);

    QAction* linkAction = addAction(makeLinkIcon(iconColor), tr("Insert Link"));
    linkAction->setToolTip(tr("Insert Link"));
    connect(linkAction, &QAction::triggered, this, &ToolBar::linkClicked);
    m_actions.append(linkAction);

    QAction* wlAction = addAction(makeTextIcon("[[ ]]", iconColor), tr("Insert Wikilink"));
    wlAction->setToolTip(tr("Insert Wikilink"));
    connect(wlAction, &QAction::triggered, this, &ToolBar::wikilinkClicked);
    m_actions.append(wlAction);

    addSeparator();

    // Font size combo box
    m_fontSizeCombo = new QComboBox(this);
    m_fontSizeCombo->setToolTip(tr("Font Size"));
    for (int size = 10; size <= 20; ++size) {
        m_fontSizeCombo->addItem(QString::number(size), size);
    }
    m_fontSizeCombo->setCurrentIndex(3); // default 13
    m_fontSizeCombo->setFixedWidth(50);
    connect(m_fontSizeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int index) {
                emit fontSizeChanged(m_fontSizeCombo->itemData(index).toInt());
            });
    addWidget(m_fontSizeCombo);
}

void ToolBar::setFontSize(int size)
{
    if (!m_fontSizeCombo) return;
    for (int i = 0; i < m_fontSizeCombo->count(); ++i) {
        if (m_fontSizeCombo->itemData(i).toInt() == size) {
            m_fontSizeCombo->setCurrentIndex(i);
            return;
        }
    }
}
