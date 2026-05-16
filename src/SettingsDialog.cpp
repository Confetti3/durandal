#include "SettingsDialog.h"
#include "AppSettings.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QFontDatabase>
#include <QPushButton>

SettingsDialog::SettingsDialog(AppSettings* settings, QWidget* parent)
    : QDialog(parent)
    , m_settings(settings)
    , m_fontFamilyCombo(new QComboBox(this))
    , m_fontSizeSpin(new QSpinBox(this))
    , m_tabWidthCombo(new QComboBox(this))
    , m_wordWrapCheck(new QCheckBox(tr("Enable word wrap"), this))
    , m_lineNumbersCheck(new QCheckBox(tr("Show line numbers"), this))
    , m_autoSaveCheck(new QCheckBox(tr("Enable auto-save"), this))
    , m_autoSaveIntervalSpin(new QSpinBox(this))
    , m_previewDelaySpin(new QSpinBox(this))
    , m_themeCombo(new QComboBox(this))
    , m_openLastFolderCheck(new QCheckBox(tr("Open last folder on startup"), this))
{
    setupUI();
    loadSettings();
}

void SettingsDialog::setupUI()
{
    setWindowTitle(tr("Settings"));
    resize(480, 420);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(20, 16, 20, 16);

    // --- Editor ---
    QGroupBox* editorGroup = new QGroupBox(tr("Editor"), this);
    QFormLayout* editorLayout = new QFormLayout(editorGroup);
    editorLayout->setSpacing(10);
    editorLayout->setContentsMargins(8, 8, 8, 8);

    // Font family: collect monospace fonts
    QFontDatabase db;
    QStringList families = db.families();
    for (const QString& family : families) {
        if (db.isFixedPitch(family)) {
            m_fontFamilyCombo->addItem(family);
        }
    }
    if (m_fontFamilyCombo->count() == 0) {
        m_fontFamilyCombo->addItems({"Consolas", "Courier New", "Lucida Console", "Monospace"});
    }
    editorLayout->addRow(tr("Font family:"), m_fontFamilyCombo);

    m_fontSizeSpin->setRange(6, 48);
    editorLayout->addRow(tr("Font size:"), m_fontSizeSpin);

    m_tabWidthCombo->addItem(tr("2 spaces"), 2);
    m_tabWidthCombo->addItem(tr("4 spaces"), 4);
    m_tabWidthCombo->addItem(tr("8 spaces"), 8);
    editorLayout->addRow(tr("Tab width:"), m_tabWidthCombo);

    editorLayout->addRow(m_wordWrapCheck);
    editorLayout->addRow(m_lineNumbersCheck);

    mainLayout->addWidget(editorGroup);

    // --- Auto-save ---
    QGroupBox* autoSaveGroup = new QGroupBox(tr("Auto-save"), this);
    QFormLayout* autoSaveLayout = new QFormLayout(autoSaveGroup);
    autoSaveLayout->setSpacing(10);
    autoSaveLayout->setContentsMargins(8, 8, 8, 8);

    autoSaveLayout->addRow(m_autoSaveCheck);

    m_autoSaveIntervalSpin->setRange(5, 300);
    m_autoSaveIntervalSpin->setSuffix(tr(" s"));
    autoSaveLayout->addRow(tr("Interval:"), m_autoSaveIntervalSpin);

    mainLayout->addWidget(autoSaveGroup);

    // --- Preview ---
    QGroupBox* previewGroup = new QGroupBox(tr("Preview"), this);
    QFormLayout* previewLayout = new QFormLayout(previewGroup);
    previewLayout->setSpacing(10);
    previewLayout->setContentsMargins(8, 8, 8, 8);

    m_previewDelaySpin->setRange(100, 2000);
    m_previewDelaySpin->setSuffix(tr(" ms"));
    previewLayout->addRow(tr("Update delay:"), m_previewDelaySpin);

    mainLayout->addWidget(previewGroup);

    // --- General ---
    QGroupBox* generalGroup = new QGroupBox(tr("General"), this);
    QFormLayout* generalLayout = new QFormLayout(generalGroup);
    generalLayout->setSpacing(10);
    generalLayout->setContentsMargins(8, 8, 8, 8);

    m_themeCombo->addItem(tr("Light"), false);
    m_themeCombo->addItem(tr("Dark"), true);
    generalLayout->addRow(tr("Theme:"), m_themeCombo);

    generalLayout->addRow(m_openLastFolderCheck);

    mainLayout->addWidget(generalGroup);

    // --- Buttons ---
    QDialogButtonBox* buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &SettingsDialog::onAccept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(buttonBox);

    mainLayout->addStretch();
}

void SettingsDialog::loadSettings()
{
    int idx = m_fontFamilyCombo->findText(m_settings->editorFontFamily());
    m_fontFamilyCombo->setCurrentIndex(idx >= 0 ? idx : 0);

    m_fontSizeSpin->setValue(m_settings->editorFontSize());

    int tabIdx = m_tabWidthCombo->findData(m_settings->editorTabWidth());
    m_tabWidthCombo->setCurrentIndex(tabIdx >= 0 ? tabIdx : 1);

    m_wordWrapCheck->setChecked(m_settings->editorWordWrap());
    m_lineNumbersCheck->setChecked(m_settings->editorShowLineNumbers());
    m_autoSaveCheck->setChecked(m_settings->autoSaveEnabled());
    m_autoSaveIntervalSpin->setValue(m_settings->autoSaveInterval());
    m_previewDelaySpin->setValue(m_settings->previewUpdateDelay());

    int themeIdx = m_themeCombo->findData(m_settings->useDarkTheme());
    m_themeCombo->setCurrentIndex(themeIdx >= 0 ? themeIdx : 0);

    m_openLastFolderCheck->setChecked(m_settings->openLastFolderOnStartup());
}

void SettingsDialog::onAccept()
{
    m_settings->setEditorFontFamily(m_fontFamilyCombo->currentText());
    m_settings->setEditorFontSize(m_fontSizeSpin->value());
    m_settings->setEditorTabWidth(m_tabWidthCombo->currentData().toInt());
    m_settings->setEditorWordWrap(m_wordWrapCheck->isChecked());
    m_settings->setEditorShowLineNumbers(m_lineNumbersCheck->isChecked());
    m_settings->setAutoSaveEnabled(m_autoSaveCheck->isChecked());
    m_settings->setAutoSaveInterval(m_autoSaveIntervalSpin->value());
    m_settings->setPreviewUpdateDelay(m_previewDelaySpin->value());
    m_settings->setUseDarkTheme(m_themeCombo->currentData().toBool());
    m_settings->setOpenLastFolderOnStartup(m_openLastFolderCheck->isChecked());
    m_settings->save();

    emit settingsApplied();
    accept();
}
