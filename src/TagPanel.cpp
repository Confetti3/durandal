#include "TagPanel.h"
#include "VaultManager.h"
#include "TagParser.h"
#include <QMenu>

TagPanel::TagPanel(VaultManager* folder, QWidget* parent)
    : QWidget(parent)
    , m_folder(folder)
    , m_parser(new TagParser)
    , m_listWidget(new QListWidget(this))
    , m_filterBox(new QLineEdit(this))
    , m_headerLabel(new QLabel(tr("Tags"), this))
{
    setupUI();
}

void TagPanel::setupUI()
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->setSpacing(4);

    m_headerLabel->setStyleSheet("font-weight: bold; padding: 4px;");
    layout->addWidget(m_headerLabel);

    m_filterBox->setPlaceholderText(tr("Filter tags..."));
    m_filterBox->setClearButtonEnabled(true);
    layout->addWidget(m_filterBox);

    m_listWidget->setAlternatingRowColors(true);
    m_listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    m_listWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    layout->addWidget(m_listWidget);

    connect(m_filterBox, &QLineEdit::textChanged,
            this, &TagPanel::onFilterChanged);
    connect(m_listWidget, &QListWidget::itemClicked,
            this, &TagPanel::onItemClicked);
    connect(m_listWidget, &QListWidget::customContextMenuRequested,
            [this](const QPoint& pos) {
        QListWidgetItem* item = m_listWidget->itemAt(pos);
        if (!item) return;

        QString tag = item->data(Qt::UserRole).toString();
        QStringList files = m_tagIndex.value(tag.toLower());

        QMenu menu;
        for (const QString& f : files) {
            menu.addAction(QFileInfo(f).fileName(), [this, f]() {
                emit fileSelectedByTag(f);
            });
        }
        menu.exec(m_listWidget->viewport()->mapToGlobal(pos));
    });
}

void TagPanel::refresh()
{
    buildTagIndex();
    populateList();
}

void TagPanel::clear()
{
    m_tagIndex.clear();
    m_listWidget->clear();
    m_headerLabel->setText(tr("Tags"));
}

void TagPanel::onItemClicked(QListWidgetItem* item)
{
    QString tag = item->data(Qt::UserRole).toString();
    if (!tag.isEmpty()) {
        emit tagSelected(tag);
    }
}

void TagPanel::onFilterChanged(const QString& text)
{
    populateList(text);
}

void TagPanel::buildTagIndex()
{
    m_tagIndex.clear();

    if (!m_folder->isOpen()) return;

    QMap<QString, QString> fileContents;
    for (const QString& f : m_folder->allFiles()) {
        fileContents[f] = m_folder->readFile(f);
    }

    m_tagIndex = m_parser->buildTagIndex(fileContents);
}

void TagPanel::populateList(const QString& filter)
{
    m_listWidget->clear();

    QStringList tags = m_tagIndex.keys();
    tags.sort(Qt::CaseInsensitive);

    int totalCount = 0;
    QMap<QString, int> tagCounts;
    for (const QString& tag : tags) {
        tagCounts[tag] = m_tagIndex[tag].size();
        totalCount += tagCounts[tag];
    }

    m_headerLabel->setText(tr("Tags (%1 tags, %2 refs)").arg(tags.size()).arg(totalCount));

    for (const QString& tag : tags) {
        if (!filter.isEmpty() && !tag.contains(filter, Qt::CaseInsensitive)) continue;

        QString label = QStringLiteral("%1  (%2)").arg(tag, QString::number(tagCounts[tag]));
        QListWidgetItem* item = new QListWidgetItem(label);
        item->setData(Qt::UserRole, tag);
        item->setToolTip(tr("%1 files").arg(tagCounts[tag]));
        m_listWidget->addItem(item);
    }
}
