#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QFormLayout>

class AppSettings;

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(AppSettings* settings, QWidget* parent = nullptr);

signals:
    void settingsApplied();

private slots:
    void onAccept();

private:
    AppSettings* m_settings;

    QComboBox* m_fontFamilyCombo;
    QSpinBox* m_fontSizeSpin;
    QComboBox* m_tabWidthCombo;
    QCheckBox* m_wordWrapCheck;
    QCheckBox* m_lineNumbersCheck;
    QCheckBox* m_autoSaveCheck;
    QSpinBox* m_autoSaveIntervalSpin;
    QSpinBox* m_previewDelaySpin;
    QComboBox* m_themeCombo;
    QCheckBox* m_openLastFolderCheck;

    void setupUI();
    void loadSettings();
};

#endif // SETTINGSDIALOG_H
