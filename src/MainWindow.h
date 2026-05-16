#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDockWidget>
#include <QMap>
#include <QString>
#include <QCloseEvent>
#include <QDragEnterEvent>
#include <QDropEvent>

class VaultManager;
class FileTreeModel;
class MarkdownRenderer;
class SearchEngine;
class WikilinkParser;
class TagParser;
class AppSettings;
class EditorWidget;
class PreviewWidget;
class FileTreePanel;
class BacklinksPanel;
class SearchPanel;
class TagPanel;
class ToolBar;
class StatusBar;
class SettingsDialog;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    void openFolderPath(const QString& path);

protected:
    void closeEvent(QCloseEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
    void onOpenFolder();
    void onCreateFolder();
    void onRecentFolderTriggered();
    void onOpenFileInEditor(const QString& relativePath);
    void onEditorContentChanged();
    void onPreviewWikilinkClicked(const QString& title);
    void onBacklinkSelected(const QString& filePath);
    void onSearchResultSelected(const QString& filePath);
    void onTagSelected(const QString& tag);
    void onTagFileSelected(const QString& filePath);
    void onToggleTheme();
    void onAbout();
    void onNewFile();
    void onSaveFile();
    void onRefreshFolder();
    void onAutoSave();
    void onToggleLeftDock();
    void onToggleRightDock();
    void onToggleEditorDock();
    void onTogglePreviewDock();
    void onResetLayout();
    void onToggleWordWrap();
    void onSettings();

private:
    // Core
    VaultManager* m_folder;
    FileTreeModel* m_fileTreeModel;
    MarkdownRenderer* m_renderer;
    SearchEngine* m_searchEngine;
    WikilinkParser* m_wikilinkParser;
    TagParser* m_tagParser;
    AppSettings* m_settings;

    // UI Widgets
    EditorWidget* m_editor;
    PreviewWidget* m_preview;
    FileTreePanel* m_fileTreePanel;
    BacklinksPanel* m_backlinksPanel;
    SearchPanel* m_searchPanel;
    TagPanel* m_tagPanel;
    ToolBar* m_toolBar;
    StatusBar* m_statusBar;

    // Layout
    QDockWidget* m_leftDock;
    QDockWidget* m_editorDock;
    QDockWidget* m_previewDock;
    QDockWidget* m_backlinksDock;
    QDockWidget* m_searchDock;
    QDockWidget* m_tagDock;

    // State
    QString m_currentFilePath;
    bool m_darkTheme;
    bool m_isLoading;
    bool m_dirty;

    // Defaults
    QByteArray m_defaultDockState;

    // Timers
    QTimer* m_autoSaveTimer;
    QTimer* m_previewDebounceTimer;

    // Menu helpers
    QMenu* m_recentFoldersMenu;
    void updateRecentFoldersMenu();

    void setupUI();
    void expandDockTabBars();
    void setupMenus();
    void setupConnections();
    void setupShortcuts();
    void setupAutoSave();
    void openFolder(const QString& path);
    void openFile(const QString& relativePath);
    void updatePreview();
    void updateEditorCompletions();
    void updateBacklinks();
    void updateTagPanel();
    void applyTheme();
    void applySettings();
    void saveWindowState();
    void restoreWindowState();
    void setWindowTitleForNote(const QString& relativePath);
    void updateSearchIndex();
    bool confirmDiscardChanges();
    void setDirty(bool dirty);
};

#endif // MAINWINDOW_H
