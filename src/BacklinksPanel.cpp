#include "BacklinksPanel.h"
#include "VaultManager.h"
#include "WikilinkParser.h"
#include <QFileInfo>

BacklinksPanel::BacklinksPanel(VaultManager* folder, QWidget* parent)
    : QWidget(parent)
    , m_folder(folder)
    , m_parser(new WikilinkParser)
    , m_listWidget(new QListWidget(this))
    , m_headerLabel(new QLabel(tr("Backlinks"), this))
{
    setupUI();
}

void BacklinksPanel::setupUI()
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->setSpacing(4);

    m_headerLabel->setStyleSheet("font-weight: bold; padding: 4px;");
    layout->addWidget(m_headerLabel);

    m_listWidget->setAlternatingRowColors(true);
    m_listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    layout->addWidget(m_listWidget);

    connect(m_listWidget, &QListWidget::itemClicked,
            this, &BacklinksPanel::onItemClicked);
}

void BacklinksPanel::updateBacklinks(const QString& notePath)
{
    m_listWidget->clear();

    if (notePath.isEmpty() || !m_folder->isOpen()) {
        m_headerLabel->setText(tr("Backlinks"));
        return;
    }

    QFileInfo fi(notePath);
    QString targetTitle = fi.completeBaseName();

    QMap<QString, QString> fileContents;
    for (const QString& f : m_folder->allFiles()) {
        if (f != notePath) {
            fileContents[f] = m_folder->readFile(f);
        }
    }

    QStringList backlinks = m_parser->findBacklinks(targetTitle, fileContents);
    m_headerLabel->setText(tr("Backlinks (%1)").arg(backlinks.size()));

    for (const QString& bl : backlinks) {
        QListWidgetItem* item = new QListWidgetItem(QFileInfo(bl).fileName());
        item->setData(Qt::UserRole, bl);
        item->setToolTip(bl);
        m_listWidget->addItem(item);
    }

    if (backlinks.isEmpty()) {
        QListWidgetItem* item = new QListWidgetItem(tr("No backlinks found"));
        item->setFlags(Qt::NoItemFlags);
        m_listWidget->addItem(item);
    }
}

void BacklinksPanel::clear()
{
    m_listWidget->clear();
    m_headerLabel->setText(tr("Backlinks"));
}

void BacklinksPanel::onItemClicked(QListWidgetItem* item)
{
    QString path = item->data(Qt::UserRole).toString();
    if (!path.isEmpty()) {
        emit backlinkSelected(path);
    }
}
