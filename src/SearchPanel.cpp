#include "SearchPanel.h"
#include "SearchEngine.h"
#include "VaultManager.h"
#include <QFileInfo>
#include <QTimer>

SearchPanel::SearchPanel(SearchEngine* engine, QWidget* parent)
    : QWidget(parent)
    , m_engine(engine)
    , m_searchBox(new QLineEdit(this))
    , m_listWidget(new QListWidget(this))
    , m_headerLabel(new QLabel(tr("Search"), this))
{
    setupUI();
}

void SearchPanel::setupUI()
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->setSpacing(4);

    m_headerLabel->setStyleSheet("font-weight: bold; padding: 4px;");
    layout->addWidget(m_headerLabel);

    m_searchBox->setPlaceholderText(tr("Search all notes..."));
    m_searchBox->setClearButtonEnabled(true);
    layout->addWidget(m_searchBox);

    m_listWidget->setAlternatingRowColors(true);
    m_listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    layout->addWidget(m_listWidget);

    auto* debounceTimer = new QTimer(this);
    debounceTimer->setSingleShot(true);
    debounceTimer->setInterval(250);

    connect(m_searchBox, &QLineEdit::textChanged, [debounceTimer](const QString&) {
        debounceTimer->start();
    });

    connect(debounceTimer, &QTimer::timeout, [this]() {
        performSearch(m_searchBox->text());
    });

    connect(m_listWidget, &QListWidget::itemClicked,
            this, &SearchPanel::onResultClicked);
}

void SearchPanel::refreshIndex()
{
    // Indexing is handled externally by MainWindow calling SearchEngine::indexFiles
}

void SearchPanel::onSearchTextChanged(const QString& text)
{
    performSearch(text);
}

void SearchPanel::onResultClicked(QListWidgetItem* item)
{
    QString path = item->data(Qt::UserRole).toString();
    if (!path.isEmpty()) {
        emit resultSelected(path);
    }
}

void SearchPanel::performSearch(const QString& query)
{
    m_listWidget->clear();

    if (query.trimmed().isEmpty()) {
        m_headerLabel->setText(tr("Search"));
        return;
    }

    QList<SearchResult> results = m_engine->search(query);
    m_headerLabel->setText(tr("Search (%1 results)").arg(results.size()));

    for (const SearchResult& result : results) {
        QString label = QStringLiteral("%1\n%2").arg(result.fileName, result.preview);
        QListWidgetItem* item = new QListWidgetItem(label);
        item->setData(Qt::UserRole, result.filePath);
        item->setToolTip(result.filePath);
        item->setSizeHint(QSize(0, 40));
        m_listWidget->addItem(item);
    }

    if (results.isEmpty() && !query.trimmed().isEmpty()) {
        QListWidgetItem* item = new QListWidgetItem(tr("No results found"));
        item->setFlags(Qt::NoItemFlags);
        m_listWidget->addItem(item);
    }
}
