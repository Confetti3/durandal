#include "ToolBar.h"
#include <QPainter>
#include <QComboBox>
#include <QApplication>

QIcon ToolBar::makeIcon(const QString& text, const QColor& color, qreal dpr, bool bold, bool italic)
{
    int pxSize = qRound(40 * dpr);
    QPixmap px(pxSize, pxSize);
    px.setDevicePixelRatio(dpr);
    px.fill(Qt::transparent);

    QPainter p(&px);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);

    QFont f = p.font();
    f.setPointSize(18);
    if (bold) f.setBold(true);
    if (italic) f.setItalic(true);
    p.setFont(f);
    p.setPen(color);
    p.drawText(px.rect(), Qt::AlignCenter, text);
    p.end();

    return QIcon(px);
}

ToolBar::ToolBar(QWidget* parent)
    : QToolBar(tr("Formatting"), parent)
{
    setMovable(false);
    setIconSize(QSize(28, 24));
    setupActions(false);
}

void ToolBar::setDark(bool dark)
{
    for (QAction* a : m_actions) {
        removeAction(a);
        delete a;
    }
    m_actions.clear();
    if (m_fontSizeCombo) {
        m_fontSizeCombo->deleteLater();
        m_fontSizeCombo = nullptr;
    }
    setupActions(dark);
}

void ToolBar::setupActions(bool dark)
{
    QColor iconColor = dark ? QColor("#cccccc") : QColor("#2a2a2a");
    qreal dpr = devicePixelRatioF();

    // Font size combo (leftmost)
    m_fontSizeCombo = new QComboBox(this);
    m_fontSizeCombo->setToolTip(tr("Font Size"));
    for (int size = 10; size <= 20; ++size) {
        m_fontSizeCombo->addItem(QString::number(size), size);
    }
    m_fontSizeCombo->setCurrentIndex(3);
    m_fontSizeCombo->setFixedWidth(50);
    connect(m_fontSizeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int index) {
                emit fontSizeChanged(m_fontSizeCombo->itemData(index).toInt());
            });
    addWidget(m_fontSizeCombo);

    addSeparator();

    QAction* boldAction = addAction(makeIcon(QStringLiteral("B"), iconColor, dpr, true), tr("Bold"));
    boldAction->setToolTip(tr("Bold (Ctrl+B)"));
    boldAction->setShortcut(QKeySequence::Bold);
    connect(boldAction, &QAction::triggered, this, &ToolBar::boldClicked);
    m_actions.append(boldAction);

    QAction* italicAction = addAction(makeIcon(QStringLiteral("I"), iconColor, dpr, false, true), tr("Italic"));
    italicAction->setToolTip(tr("Italic (Ctrl+I)"));
    italicAction->setShortcut(QKeySequence::Italic);
    connect(italicAction, &QAction::triggered, this, &ToolBar::italicClicked);
    m_actions.append(italicAction);

    addSeparator();

    QAction* h1Action = addAction(makeIcon(QStringLiteral("H\u2081"), iconColor, dpr, true), tr("Heading 1"));
    h1Action->setToolTip(tr("Heading 1"));
    connect(h1Action, &QAction::triggered, this, &ToolBar::heading1Clicked);
    m_actions.append(h1Action);

    QAction* h2Action = addAction(makeIcon(QStringLiteral("H\u2082"), iconColor, dpr, true), tr("Heading 2"));
    h2Action->setToolTip(tr("Heading 2"));
    connect(h2Action, &QAction::triggered, this, &ToolBar::heading2Clicked);
    m_actions.append(h2Action);

    QAction* h3Action = addAction(makeIcon(QStringLiteral("H\u2083"), iconColor, dpr, true), tr("Heading 3"));
    h3Action->setToolTip(tr("Heading 3"));
    connect(h3Action, &QAction::triggered, this, &ToolBar::heading3Clicked);
    m_actions.append(h3Action);

    addSeparator();

    QAction* ulAction = addAction(makeIcon(QStringLiteral("\u2022"), iconColor, dpr), tr("Bullet List"));
    ulAction->setToolTip(tr("Bullet List"));
    connect(ulAction, &QAction::triggered, this, &ToolBar::bulletListClicked);
    m_actions.append(ulAction);

    QAction* olAction = addAction(makeIcon(QStringLiteral("1."), iconColor, dpr), tr("Numbered List"));
    olAction->setToolTip(tr("Numbered List"));
    connect(olAction, &QAction::triggered, this, &ToolBar::numberedListClicked);
    m_actions.append(olAction);

    addSeparator();

    QAction* codeAction = addAction(makeIcon(QStringLiteral("</>"), iconColor, dpr), tr("Code Block"));
    codeAction->setToolTip(tr("Code Block"));
    connect(codeAction, &QAction::triggered, this, &ToolBar::codeBlockClicked);
    m_actions.append(codeAction);

    QAction* linkAction = addAction(makeIcon(QStringLiteral("\u2197"), iconColor, dpr), tr("Insert Link"));
    linkAction->setToolTip(tr("Insert Link"));
    connect(linkAction, &QAction::triggered, this, &ToolBar::linkClicked);
    m_actions.append(linkAction);

    QAction* wlAction = addAction(makeIcon(QStringLiteral("[[ ]]"), iconColor, dpr), tr("Insert Wikilink"));
    wlAction->setToolTip(tr("Insert Wikilink"));
    connect(wlAction, &QAction::triggered, this, &ToolBar::wikilinkClicked);
    m_actions.append(wlAction);
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
