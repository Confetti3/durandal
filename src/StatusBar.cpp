#include "StatusBar.h"
#include <QHBoxLayout>

StatusBar::StatusBar(QWidget* parent)
    : QStatusBar(parent)
    , m_lineColLabel(new QLabel(this))
    , m_wordCountLabel(new QLabel(this))
    , m_fileTypeLabel(new QLabel(this))
    , m_modifiedLabel(new QLabel(this))
    , m_fileNameLabel(new QLabel(this))
{
    setupUI();
}

void StatusBar::setupUI()
{
    // Terminal-style prompt label
    QLabel* promptLabel = new QLabel(">", this);
    promptLabel->setStyleSheet("color: #0dbc79; font-weight: bold; padding-right: 4px;");
    addWidget(promptLabel, 0);

    m_lineColLabel->setMinimumWidth(160);
    m_wordCountLabel->setMinimumWidth(120);
    m_fileNameLabel->setMinimumWidth(200);

    addWidget(m_lineColLabel, 0);
    addWidget(m_wordCountLabel, 0);
    addWidget(m_fileTypeLabel, 0);
    addWidget(m_modifiedLabel, 0);
    addPermanentWidget(m_fileNameLabel);

    m_lineColLabel->setStyleSheet("padding: 0 8px; font-size: 12px;");
    m_wordCountLabel->setStyleSheet("padding: 0 8px; font-size: 12px;");
    m_fileTypeLabel->setStyleSheet("padding: 0 8px; font-size: 12px; color: #888888;");
    m_modifiedLabel->setStyleSheet("padding: 0 4px; font-size: 12px; color: #f0a500;");
    m_fileNameLabel->setStyleSheet("padding: 0 8px; font-size: 12px;");

    clear();
}

void StatusBar::setCursorPosition(int line, int column)
{
    m_lineColLabel->setText(tr("Ln %1, Col %2").arg(line).arg(column));
}

void StatusBar::setWordCount(int count)
{
    m_wordCountLabel->setText(tr("Words: %1").arg(count));
}

void StatusBar::setFileName(const QString& name)
{
    m_fileNameLabel->setText(name);
}

void StatusBar::setFileType(const QString& type)
{
    m_fileTypeLabel->setText(type);
}

void StatusBar::setModified(bool modified)
{
    m_modifiedLabel->setText(modified ? QStringLiteral("\u25CF") : QString());
}

void StatusBar::clear()
{
    m_lineColLabel->setText(tr("Ln 1, Col 1"));
    m_wordCountLabel->setText(tr("Words: 0"));
    m_fileTypeLabel->clear();
    m_modifiedLabel->clear();
    m_fileNameLabel->setText(tr("No file open"));
}
