#include "MainWindow.h"

#include "VaultManager.h"
#include "FileTreeModel.h"
#include "MarkdownRenderer.h"
#include "SearchEngine.h"
#include "WikilinkParser.h"
#include "TagParser.h"
#include "AppSettings.h"
#include "EditorWidget.h"
#include "PreviewWidget.h"
#include "FileTreePanel.h"
#include "BacklinksPanel.h"
#include "SearchPanel.h"
#include "TagPanel.h"
#include "ToolBar.h"
#include "StatusBar.h"
#include "Version.h"

#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QApplication>
#include <QStyleFactory>
#include <QTimer>
#include <QLabel>
#include <QShortcut>
#include <QFile>
#include <QDataStream>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>
#include <QDir>
#include <QTextStream>
#include <QFontInfo>
#include <QMouseEvent>
#include <QToolBar>
#include <QWindow>
#include <QTabBar>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_folder(new VaultManager(this))
    , m_fileTreeModel(nullptr)
    , m_renderer(new MarkdownRenderer)
    , m_searchEngine(new SearchEngine(this))
    , m_wikilinkParser(new WikilinkParser)
    , m_tagParser(new TagParser)
    , m_settings(new AppSettings)
    , m_editor(new EditorWidget(this))
    , m_preview(new PreviewWidget(this))
    , m_fileTreePanel(nullptr)
    , m_backlinksPanel(new BacklinksPanel(m_folder, this))
    , m_searchPanel(new SearchPanel(m_searchEngine, this))
    , m_tagPanel(new TagPanel(m_folder, this))
    , m_toolBar(new ToolBar(this))
    , m_statusBar(new StatusBar(this))
    , m_leftDock(nullptr)
    , m_editorDock(nullptr)
    , m_previewDock(nullptr)
    , m_backlinksDock(nullptr)
    , m_searchDock(nullptr)
    , m_tagDock(nullptr)
    , m_darkTheme(false)
    , m_isLoading(false)
    , m_dirty(false)
    , m_recentFoldersMenu(nullptr)
{
    m_settings->load();
    m_darkTheme = m_settings->useDarkTheme();

    // Apply saved editor font size
    int fontSize = m_settings->editorFontSize();
    m_editor->setFontSize(fontSize);
    m_toolBar->setFontSize(fontSize);

    // Set global monospace font
    QFont monoFont("Cascadia Code", 13);
    if (!QFontInfo(monoFont).exactMatch()) {
        monoFont.setFamily("Consolas");
    }
    if (!QFontInfo(monoFont).exactMatch()) {
        monoFont.setFamily("Courier New");
    }
    monoFont.setStyleHint(QFont::Monospace);
    qApp->setFont(monoFont);

    setupUI();
    setupMenus();
    setupConnections();
    setupAutoSave();
    setupShortcuts();
    applyTheme();
    restoreWindowState();
    m_defaultDockState = saveState();

    setWindowTitle("Durandal");
    resize(1200, 800);
    setMinimumSize(800, 600);

    // Show a welcome message in preview
    QString welcome = tr(
        "# Welcome to Durandal\n\n"
        "A cross-platform note-taking app.\n\n"
        "**To get started:**\n"
        "- Click **File &rarr; Open Folder** to open a folder\n"
        "- **File &rarr; Create New Folder** to start fresh\n"
        "- Or drag-and-drop any folder onto this window\n\n"
        "**Features:**\n"
        "- Markdown & HTML editing with syntax highlighting\n"
        "- Live preview with [[wikilink]] navigation\n"
        "- Backlinks, tags, and full-text search\n"
        "- Light & dark themes\n"
        "- Auto-save every 30 seconds\n"
    );
    m_preview->setMarkdown(welcome, m_darkTheme);
}

MainWindow::~MainWindow()
{
    delete m_renderer;
    delete m_wikilinkParser;
    delete m_tagParser;
    delete m_settings;
}

void MainWindow::setupUI()
{
    // --- Left sidebar: Files ---
    m_leftDock = new QDockWidget(tr("Files"), this);
    m_leftDock->setObjectName("FilesDock");
    m_leftDock->setFeatures(QDockWidget::DockWidgetMovable |
                             QDockWidget::DockWidgetFloatable |
                             QDockWidget::DockWidgetClosable);
    m_fileTreePanel = new FileTreePanel(m_folder, m_leftDock);
    m_leftDock->setWidget(m_fileTreePanel);
    m_leftDock->setMinimumWidth(180);
    addDockWidget(Qt::LeftDockWidgetArea, m_leftDock);

    // --- Main workspace (Right dock area) ---
    // Editor first
    m_editorDock = new QDockWidget(tr("Editor"), this);
    m_editorDock->setObjectName("EditorDock");
    m_editorDock->setFeatures(QDockWidget::DockWidgetMovable |
                               QDockWidget::DockWidgetFloatable |
                               QDockWidget::DockWidgetClosable);
    m_editorDock->setWidget(m_editor);
    m_editorDock->setMinimumWidth(320);
    addDockWidget(Qt::RightDockWidgetArea, m_editorDock);

    // Preview — split to the right of Editor
    m_previewDock = new QDockWidget(tr("Preview"), this);
    m_previewDock->setObjectName("PreviewDock");
    m_previewDock->setFeatures(QDockWidget::DockWidgetMovable |
                                QDockWidget::DockWidgetFloatable |
                                QDockWidget::DockWidgetClosable);
    m_previewDock->setWidget(m_preview);
    m_previewDock->setMinimumWidth(300);
    addDockWidget(Qt::RightDockWidgetArea, m_previewDock);
    splitDockWidget(m_editorDock, m_previewDock, Qt::Horizontal);

    // Panel docks: Backlinks, Search, Tags — tabified on the far right
    m_backlinksDock = new QDockWidget(tr("Backlinks"), this);
    m_backlinksDock->setObjectName("BacklinksDock");
    m_backlinksDock->setFeatures(QDockWidget::DockWidgetMovable |
                                    QDockWidget::DockWidgetFloatable |
                                    QDockWidget::DockWidgetClosable);
    m_backlinksDock->setWidget(m_backlinksPanel);
    m_backlinksDock->setMinimumWidth(300);

    m_searchDock = new QDockWidget(tr("Search"), this);
    m_searchDock->setObjectName("SearchDock");
    m_searchDock->setFeatures(QDockWidget::DockWidgetMovable |
                               QDockWidget::DockWidgetFloatable |
                               QDockWidget::DockWidgetClosable);
    m_searchDock->setWidget(m_searchPanel);
    m_searchDock->setMinimumWidth(300);

    m_tagDock = new QDockWidget(tr("Tags"), this);
    m_tagDock->setObjectName("TagsDock");
    m_tagDock->setFeatures(QDockWidget::DockWidgetMovable |
                            QDockWidget::DockWidgetFloatable |
                            QDockWidget::DockWidgetClosable);
    m_tagDock->setWidget(m_tagPanel);
    m_tagDock->setMinimumWidth(300);

    addDockWidget(Qt::RightDockWidgetArea, m_backlinksDock);
    splitDockWidget(m_previewDock, m_backlinksDock, Qt::Horizontal);

    addDockWidget(Qt::RightDockWidgetArea, m_searchDock);
    tabifyDockWidget(m_backlinksDock, m_searchDock);

    addDockWidget(Qt::RightDockWidgetArea, m_tagDock);
    tabifyDockWidget(m_backlinksDock, m_tagDock);

    // Default active tabs
    m_previewDock->raise();
    m_searchDock->raise();

    // Enable tabbing, nested splits and group dragging for proper snapping
    setDockOptions(AnimatedDocks | AllowNestedDocks | AllowTabbedDocks | GroupedDragging);
    setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
    setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

    // Minimal central placeholder
    QWidget* centralPlaceholder = new QWidget(this);
    centralPlaceholder->setFixedSize(0, 0);
    setCentralWidget(centralPlaceholder);

    // Toolbar
    addToolBar(m_toolBar);
    if (m_toolBar->layout()) {
        m_toolBar->layout()->setSpacing(0);
        m_toolBar->layout()->setContentsMargins(0, 0, 0, 0);
    }

    // Enable window drag from toolbar and menu bar empty areas
    menuBar()->installEventFilter(this);
    m_toolBar->installEventFilter(this);

    // Status bar
    setStatusBar(m_statusBar);

    // Enable drag-and-drop for folders
    setAcceptDrops(true);

    // Delay so internal dock tab bars are created before we configure them
    QTimer::singleShot(0, this, &MainWindow::expandDockTabBars);
}

void MainWindow::expandDockTabBars()
{
    for (QTabBar* tabBar : findChildren<QTabBar*>()) {
        tabBar->setExpanding(true);
        tabBar->setUsesScrollButtons(false);
        tabBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    }
}

void MainWindow::setupMenus()
{
    // File menu
    QMenu* fileMenu = menuBar()->addMenu(tr("&File"));

    QAction* openFolderAction = fileMenu->addAction(tr("&Open Folder..."));
    openFolderAction->setShortcut(QKeySequence("Ctrl+Shift+O"));
    connect(openFolderAction, &QAction::triggered, this, &MainWindow::onOpenFolder);

    QAction* createFolderAction = fileMenu->addAction(tr("&New Folder"));
    createFolderAction->setShortcut(QKeySequence("Ctrl+Shift+N"));
    connect(createFolderAction, &QAction::triggered, this, &MainWindow::onCreateFolder);

    fileMenu->addSeparator();

    m_recentFoldersMenu = fileMenu->addMenu(tr("&Recent Folders"));
    updateRecentFoldersMenu();

    fileMenu->addSeparator();

    QAction* newFileAction = fileMenu->addAction(tr("&New Note"));
    newFileAction->setShortcut(QKeySequence::New);
    connect(newFileAction, &QAction::triggered, this, &MainWindow::onNewFile);

    QAction* saveAction = fileMenu->addAction(tr("&Save"));
    saveAction->setShortcut(QKeySequence::Save);
    connect(saveAction, &QAction::triggered, this, &MainWindow::onSaveFile);

    fileMenu->addSeparator();

    QAction* quitAction = fileMenu->addAction(tr("&Quit"));
    quitAction->setShortcut(QKeySequence::Quit);
    connect(quitAction, &QAction::triggered, this, &QWidget::close);

    // Edit menu
    QMenu* editMenu = menuBar()->addMenu(tr("&Edit"));

    QAction* undoAction = editMenu->addAction(tr("&Undo"));
    undoAction->setShortcut(QKeySequence::Undo);
    connect(undoAction, &QAction::triggered, m_editor, &QPlainTextEdit::undo);

    QAction* redoAction = editMenu->addAction(tr("&Redo"));
    redoAction->setShortcut(QKeySequence::Redo);
    connect(redoAction, &QAction::triggered, m_editor, &QPlainTextEdit::redo);

    editMenu->addSeparator();

    QAction* cutAction = editMenu->addAction(tr("Cu&t"));
    cutAction->setShortcut(QKeySequence::Cut);
    connect(cutAction, &QAction::triggered, m_editor, &QPlainTextEdit::cut);

    QAction* copyAction = editMenu->addAction(tr("&Copy"));
    copyAction->setShortcut(QKeySequence::Copy);
    connect(copyAction, &QAction::triggered, m_editor, &QPlainTextEdit::copy);

    QAction* pasteAction = editMenu->addAction(tr("&Paste"));
    pasteAction->setShortcut(QKeySequence::Paste);
    connect(pasteAction, &QAction::triggered, m_editor, &QPlainTextEdit::paste);

    // View menu
    QMenu* viewMenu = menuBar()->addMenu(tr("&View"));

    QAction* toggleThemeAction = viewMenu->addAction(tr("Toggle &Dark Theme"));
    toggleThemeAction->setShortcut(QKeySequence("Ctrl+T"));
    connect(toggleThemeAction, &QAction::triggered, this, &MainWindow::onToggleTheme);

    viewMenu->addSeparator();

    QAction* toggleLeftAction = viewMenu->addAction(tr("Toggle &Files Panel"));
    toggleLeftAction->setShortcut(QKeySequence("Ctrl+B"));
    toggleLeftAction->setCheckable(true);
    toggleLeftAction->setChecked(true);
    connect(toggleLeftAction, &QAction::triggered, this, &MainWindow::onToggleLeftDock);
    connect(m_leftDock, &QDockWidget::visibilityChanged, toggleLeftAction, &QAction::setChecked);

    QAction* toggleRightAction = viewMenu->addAction(tr("Toggle &Panels"));
    toggleRightAction->setShortcut(QKeySequence("Ctrl+J"));
    toggleRightAction->setCheckable(true);
    toggleRightAction->setChecked(true);
    connect(toggleRightAction, &QAction::triggered, this, &MainWindow::onToggleRightDock);

    auto updateRightToggle = [this, toggleRightAction]() {
        bool any = m_backlinksDock->isVisible() || m_searchDock->isVisible() || m_tagDock->isVisible();
        toggleRightAction->setChecked(any);
    };
    connect(m_backlinksDock, &QDockWidget::visibilityChanged, this, updateRightToggle);
    connect(m_searchDock, &QDockWidget::visibilityChanged, this, updateRightToggle);
    connect(m_tagDock, &QDockWidget::visibilityChanged, this, updateRightToggle);

    viewMenu->addSeparator();

    QAction* toggleEditorAction = viewMenu->addAction(tr("Toggle &Editor"));
    toggleEditorAction->setShortcut(QKeySequence("Ctrl+E"));
    toggleEditorAction->setCheckable(true);
    toggleEditorAction->setChecked(true);
    connect(toggleEditorAction, &QAction::triggered, this, &MainWindow::onToggleEditorDock);
    connect(m_editorDock, &QDockWidget::visibilityChanged, toggleEditorAction, &QAction::setChecked);

    QAction* togglePreviewAction = viewMenu->addAction(tr("Toggle &Preview"));
    togglePreviewAction->setShortcut(QKeySequence("Ctrl+P"));
    togglePreviewAction->setCheckable(true);
    togglePreviewAction->setChecked(true);
    connect(togglePreviewAction, &QAction::triggered, this, &MainWindow::onTogglePreviewDock);
    connect(m_previewDock, &QDockWidget::visibilityChanged, togglePreviewAction, &QAction::setChecked);

    viewMenu->addSeparator();

    QAction* refreshAction = viewMenu->addAction(tr("&Refresh Folder"));
    refreshAction->setShortcut(QKeySequence::Refresh);
    connect(refreshAction, &QAction::triggered, this, &MainWindow::onRefreshFolder);

    viewMenu->addSeparator();

    QAction* resetLayoutAction = viewMenu->addAction(tr("Reset &Layout"));
    resetLayoutAction->setShortcut(QKeySequence("Ctrl+0"));
    connect(resetLayoutAction, &QAction::triggered, this, &MainWindow::onResetLayout);

    // Help menu
    QMenu* helpMenu = menuBar()->addMenu(tr("&Help"));
    QAction* aboutAction = helpMenu->addAction(tr("&About"));
    connect(aboutAction, &QAction::triggered, this, &MainWindow::onAbout);
}

void MainWindow::setupConnections()
{
    // Toolbar
    connect(m_toolBar, &ToolBar::boldClicked, m_editor, &EditorWidget::insertBold);
    connect(m_toolBar, &ToolBar::italicClicked, m_editor, &EditorWidget::insertItalic);
    connect(m_toolBar, &ToolBar::heading1Clicked, [this]() { m_editor->insertHeading(1); });
    connect(m_toolBar, &ToolBar::heading2Clicked, [this]() { m_editor->insertHeading(2); });
    connect(m_toolBar, &ToolBar::heading3Clicked, [this]() { m_editor->insertHeading(3); });
    connect(m_toolBar, &ToolBar::bulletListClicked, m_editor, &EditorWidget::insertBulletList);
    connect(m_toolBar, &ToolBar::numberedListClicked, m_editor, &EditorWidget::insertNumberedList);
    connect(m_toolBar, &ToolBar::codeBlockClicked, m_editor, &EditorWidget::insertCodeBlock);
    connect(m_toolBar, &ToolBar::linkClicked, m_editor, &EditorWidget::insertLink);
    connect(m_toolBar, &ToolBar::wikilinkClicked, m_editor, &EditorWidget::insertWikilink);
    connect(m_toolBar, &ToolBar::fontSizeChanged, m_editor, &EditorWidget::setFontSize);
    connect(m_editor, &EditorWidget::fontSizeChanged, m_toolBar, &ToolBar::setFontSize);

    // Editor content change -> update preview
    connect(m_editor, &QPlainTextEdit::textChanged, this, &MainWindow::onEditorContentChanged);

    // Editor cursor position -> update status bar
    connect(m_editor, &EditorWidget::cursorPositionChanged, this, [this](int line, int col) {
        m_statusBar->setCursorPosition(line, col);
        m_statusBar->setWordCount(m_editor->wordCount());
    });

    // File tree -> open file
    connect(m_fileTreePanel, &FileTreePanel::fileSelected,
            this, &MainWindow::onOpenFileInEditor);

    // Preview wikilink navigation
    connect(m_preview, &PreviewWidget::wikilinkClicked,
            this, &MainWindow::onPreviewWikilinkClicked);

    // Backlinks
    connect(m_backlinksPanel, &BacklinksPanel::backlinkSelected,
            this, &MainWindow::onBacklinkSelected);

    // Search
    connect(m_searchPanel, &SearchPanel::resultSelected,
            this, &MainWindow::onSearchResultSelected);

    // Tags
    connect(m_tagPanel, &TagPanel::tagSelected,
            this, &MainWindow::onTagSelected);
    connect(m_tagPanel, &TagPanel::fileSelectedByTag,
            this, &MainWindow::onTagFileSelected);

    // Folder events
    connect(m_folder, &VaultManager::folderOpened, this, [this](const QString&) {
        onSaveFile(); // save current file if any
        if (m_fileTreeModel) m_fileTreeModel->refresh();
        updateEditorCompletions();
        updateSearchIndex();
        updateTagPanel();
        setWindowTitle("Durandal - " + m_folder->folderName());
        m_settings->setLastFolderPath(m_folder->folderPath());
        m_settings->addRecentFolder(m_folder->folderPath());
        m_settings->save();
        updateRecentFoldersMenu();
        m_fileTreePanel->refresh();
    });

    connect(m_folder, &VaultManager::folderClosed, this, [this]() {
        if (m_fileTreeModel) m_fileTreeModel->refresh();
        m_backlinksPanel->clear();
        m_tagPanel->clear();
        m_searchEngine->clear();
        m_editor->clear();
        m_preview->setMarkdown("", m_darkTheme);
        m_statusBar->clear();
        setWindowTitle("Durandal");
    });

    connect(m_folder, &VaultManager::folderChanged, this, &MainWindow::onRefreshFolder);

    connect(m_folder, &VaultManager::fileCreated, this, [this](const QString&) {
        if (m_fileTreeModel) m_fileTreeModel->refresh();
        updateEditorCompletions();
    });

    connect(m_folder, &VaultManager::fileDeleted, this, [this](const QString& path) {
        if (m_fileTreeModel) m_fileTreeModel->refresh();
        updateEditorCompletions();
        if (path == m_currentFilePath) {
            m_currentFilePath.clear();
            m_editor->clear();
            m_preview->setMarkdown("", m_darkTheme);
            m_statusBar->clear();
        }
    });

    connect(m_folder, &VaultManager::fileRenamed, this, [this](const QString& oldPath,
                                                                const QString& newPath) {
        if (m_fileTreeModel) m_fileTreeModel->refresh();
        updateEditorCompletions();
        if (oldPath == m_currentFilePath) {
            m_currentFilePath = newPath;
            m_editor->setFileName(newPath);
            m_statusBar->setFileName(QFileInfo(newPath).fileName());
        }
    });

    connect(m_folder, &VaultManager::fileMoved, this, [this](const QString& oldPath,
                                                             const QString& newPath) {
        if (m_fileTreeModel) m_fileTreeModel->refresh();
        updateEditorCompletions();
        if (oldPath == m_currentFilePath) {
            m_currentFilePath = newPath;
            m_editor->setFileName(newPath);
            m_statusBar->setFileName(QFileInfo(newPath).fileName());
        }
    });
}

void MainWindow::setupShortcuts()
{
    // Ctrl+B: Bold (already handled by toolbar action shortcut)
    // Ctrl+I: Italic (already handled by toolbar action shortcut)
    // Esc: move focus to editor
    QShortcut* escShortcut = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(escShortcut, &QShortcut::activated, [this]() {
        m_editor->setFocus();
    });
}

void MainWindow::openFolderPath(const QString& path)
{
    openFolder(path);
}

void MainWindow::openFolder(const QString& path)
{
    m_folder->openFolder(path);

    if (m_fileTreeModel) {
        delete m_fileTreeModel;
    }

    m_fileTreeModel = new FileTreeModel(m_folder, this);
    m_fileTreeModel->setRootPath(path);
    m_fileTreePanel->setModel(m_fileTreeModel);
    m_fileTreePanel->refresh();
}

void MainWindow::onOpenFolder()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Folder"),
                                                     m_settings->lastFolderPath());
    if (!dir.isEmpty()) {
        openFolder(dir);
    }
}

void MainWindow::onOpenFileInEditor(const QString& relativePath)
{
    openFile(relativePath);
}

void MainWindow::openFile(const QString& relativePath)
{
    if (m_isLoading) return;

    // Save current file
    if (!m_currentFilePath.isEmpty()) {
        onSaveFile();
    }

    m_isLoading = true;

    m_currentFilePath = relativePath;
    QString content = m_folder->readFile(relativePath);

    m_editor->blockSignals(true);
    m_editor->setPlainText(content);
    m_editor->setFileName(relativePath);
    m_editor->blockSignals(false);

    setDirty(false);
    updatePreview();
    updateBacklinks();
    updateEditorCompletions();
    setWindowTitleForNote(relativePath);

    m_statusBar->setFileName(QFileInfo(relativePath).fileName());
    m_statusBar->setCursorPosition(1, 1);
    m_statusBar->setWordCount(m_editor->wordCount());

    // Select in file tree
    m_fileTreePanel->selectFile(relativePath);

    m_isLoading = false;
}

void MainWindow::onEditorContentChanged()
{
    if (m_isLoading) return;
    setDirty(true);

    // Debounced update for preview, backlinks and tags
    static QTimer* debounceTimer = nullptr;
    if (!debounceTimer) {
        debounceTimer = new QTimer(this);
        debounceTimer->setSingleShot(true);
        debounceTimer->setInterval(300);
        connect(debounceTimer, &QTimer::timeout, this, [this]() {
            updatePreview();
            updateBacklinks();
            updateSearchIndex();
            updateTagPanel();
        });
    }
    debounceTimer->start();
}

void MainWindow::updatePreview()
{
    QString text = m_editor->toPlainText();
    if (m_currentFilePath.endsWith(".html", Qt::CaseInsensitive)) {
        m_preview->setHtmlContent(text, m_darkTheme);
    } else {
        m_preview->setMarkdown(text, m_darkTheme);
    }
}

void MainWindow::updateEditorCompletions()
{
    if (!m_folder->isOpen()) return;

    QStringList titles = m_folder->allNoteTitles();
    m_editor->setWikilinks(titles);

    // Collect all unique tags from folder for completer
    QSet<QString> allTags;
    for (const QString& f : m_folder->allFiles()) {
        QSet<QString> tags = m_tagParser->extractUniqueTags(m_folder->readFile(f));
        allTags.unite(tags);
    }
    m_editor->setTags(allTags.values());
}

void MainWindow::updateBacklinks()
{
    if (!m_folder->isOpen() || m_currentFilePath.isEmpty()) {
        m_backlinksPanel->clear();
        return;
    }
    m_backlinksPanel->updateBacklinks(m_currentFilePath);
}

void MainWindow::updateTagPanel()
{
    if (!m_folder->isOpen()) return;
    m_tagPanel->refresh();
}

void MainWindow::onPreviewWikilinkClicked(const QString& title)
{
    QString filePath = m_folder->resolveWikilink(title);
    if (!filePath.isEmpty()) {
        openFile(filePath);
    } else {
        // Ask user if they want to create the note
        int ret = QMessageBox::question(this, tr("Note Not Found"),
                                         tr("Note '%1' does not exist. Create it?").arg(title),
                                         QMessageBox::Yes | QMessageBox::No);
        if (ret == QMessageBox::Yes) {
            QString newFileName = title + ".md";
            if (m_folder->createFile("", newFileName)) {
                m_folder->writeFile(newFileName,
                                   QStringLiteral("# %1\n\n").arg(title));
                m_fileTreeModel->refresh();
                updateEditorCompletions();
                openFile(newFileName);
            }
        }
    }
}

void MainWindow::onBacklinkSelected(const QString& filePath)
{
    openFile(filePath);
}

void MainWindow::onSearchResultSelected(const QString& filePath)
{
    openFile(filePath);
}

void MainWindow::onTagSelected(const QString& tag)
{
    m_backlinksDock->show();
    m_searchDock->show();
    m_tagDock->show();
    m_searchDock->raise();
    Q_UNUSED(tag)
}

void MainWindow::onTagFileSelected(const QString& filePath)
{
    openFile(filePath);
}

void MainWindow::onToggleTheme()
{
    m_darkTheme = !m_darkTheme;
    m_settings->setUseDarkTheme(m_darkTheme);
    m_settings->save();
    applyTheme();
}

void MainWindow::applyTheme()
{
    QString themePath = m_darkTheme ? ":/themes/dark.qss" : ":/themes/light.qss";
    QFile themeFile(themePath);
    if (themeFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString styleSheet = themeFile.readAll();
        qApp->setStyleSheet(styleSheet);
        themeFile.close();
    }

    m_preview->setTheme(m_darkTheme);
    m_toolBar->setDark(m_darkTheme);

    // Refresh editor highlight/line-number colors for new palette
    if (m_editor) {
        m_editor->highlightCurrentLine();
        m_editor->update();
    }
}

void MainWindow::onAbout()
{
    QString aboutText = QStringLiteral(
        "<h2>Durandal %1</h2>"
        "<p>A cross-platform note-taking application.</p>"
        "<p>Built with Qt 6 and C++17.</p>"
        "<p>Features: Markdown editing, wikilinks, "
        "backlinks, full-text search, tags, and more.</p>"
    ).arg(QStringLiteral(DURANDAL_VERSION));
    QMessageBox::about(this, tr("About Durandal"), aboutText);
}

void MainWindow::onRefreshFolder()
{
    if (!m_folder->isOpen()) return;
    if (m_fileTreeModel) m_fileTreeModel->refresh();
    updateEditorCompletions();
    updateSearchIndex();
    updateTagPanel();
    updateBacklinks();
    m_statusBar->showMessage(tr("Folder refreshed."), 2000);
}

void MainWindow::onToggleLeftDock()
{
    m_leftDock->setVisible(!m_leftDock->isVisible());
}

void MainWindow::onToggleRightDock()
{
    bool anyVisible = m_backlinksDock->isVisible() || m_searchDock->isVisible() || m_tagDock->isVisible();
    m_backlinksDock->setVisible(!anyVisible);
    m_searchDock->setVisible(!anyVisible);
    m_tagDock->setVisible(!anyVisible);
}

void MainWindow::onToggleEditorDock()
{
    m_editorDock->setVisible(!m_editorDock->isVisible());
}

void MainWindow::onTogglePreviewDock()
{
    m_previewDock->setVisible(!m_previewDock->isVisible());
}

void MainWindow::onResetLayout()
{
    // Show all docks first, then restore default state
    m_leftDock->show();
    m_editorDock->show();
    m_previewDock->show();
    m_backlinksDock->show();
    m_searchDock->show();
    m_tagDock->show();
    restoreState(m_defaultDockState);
    QTimer::singleShot(0, this, &MainWindow::expandDockTabBars);
}

void MainWindow::onNewFile()
{
    if (!m_folder->isOpen()) {
        QMessageBox::information(this, tr("No Folder Open"),
                                  tr("Please open a folder first."));
        return;
    }
    m_fileTreePanel->onNewFile();
}

void MainWindow::onSaveFile()
{
    if (m_currentFilePath.isEmpty() || !m_folder->isOpen()) return;
    m_folder->writeFile(m_currentFilePath, m_editor->toPlainText());
    setDirty(false);
}

void MainWindow::onAutoSave()
{
    if (m_dirty && !m_currentFilePath.isEmpty() && m_folder->isOpen()) {
        m_folder->writeFile(m_currentFilePath, m_editor->toPlainText());
        setDirty(false);
        m_statusBar->showMessage(tr("Auto-saved."), 1500);
    }
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (!confirmDiscardChanges()) {
        event->ignore();
        return;
    }
    onSaveFile();
    saveWindowState();
    m_settings->save();
    event->accept();
}

void MainWindow::updateSearchIndex()
{
    if (!m_folder->isOpen()) return;
    QMap<QString, QString> fileContents;
    for (const QString& f : m_folder->allFiles()) {
        fileContents[f] = m_folder->readFile(f);
    }
    m_searchEngine->indexFiles(fileContents);
}

void MainWindow::saveWindowState()
{
    m_settings->setWindowGeometry(saveGeometry());
    m_settings->setWindowState(saveState());
    m_settings->setEditorFontSize(m_editor->fontSize());
}

void MainWindow::restoreWindowState()
{
    if (!m_settings->windowGeometry().isEmpty()) {
        restoreGeometry(m_settings->windowGeometry());
    }
    if (!m_settings->windowState().isEmpty()) {
        restoreState(m_settings->windowState());
    }
}

bool MainWindow::confirmDiscardChanges()
{
    if (!m_dirty) return true;
    int ret = QMessageBox::warning(this, tr("Unsaved Changes"),
        tr("The current note has unsaved changes. Save before closing?"),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    if (ret == QMessageBox::Save) {
        onSaveFile();
        return true;
    }
    return ret == QMessageBox::Discard;
}

void MainWindow::setDirty(bool dirty)
{
    if (m_dirty == dirty) return;
    m_dirty = dirty;
    if (!m_currentFilePath.isEmpty()) {
        setWindowTitleForNote(m_currentFilePath);
    }
}

void MainWindow::setupAutoSave()
{
    QTimer* autoSaveTimer = new QTimer(this);
    autoSaveTimer->setInterval(30000); // 30 seconds
    connect(autoSaveTimer, &QTimer::timeout, this, &MainWindow::onAutoSave);
    autoSaveTimer->start();
}

void MainWindow::updateRecentFoldersMenu()
{
    if (!m_recentFoldersMenu) return;
    m_recentFoldersMenu->clear();
        QStringList recent = m_settings->recentFolders();
    if (recent.isEmpty()) {
        QAction* noneAction = m_recentFoldersMenu->addAction(tr("No recent folders"));
        noneAction->setEnabled(false);
        return;
    }
    for (const QString& path : recent) {
        QAction* action = m_recentFoldersMenu->addAction(QDir(path).dirName());
        action->setToolTip(path);
        action->setData(path);
        connect(action, &QAction::triggered, this, &MainWindow::onRecentFolderTriggered);
    }
    m_recentFoldersMenu->addSeparator();
    QAction* clearAction = m_recentFoldersMenu->addAction(tr("Clear Recent Folders"));
    connect(clearAction, &QAction::triggered, this, [this]() {
        m_settings->setRecentFolders(QStringList());
        m_settings->save();
        updateRecentFoldersMenu();
    });
}

void MainWindow::onRecentFolderTriggered()
{
    QAction* action = qobject_cast<QAction*>(sender());
    if (!action) return;
    QString path = action->data().toString();
    if (QDir(path).exists()) {
        openFolder(path);
    } else {
        QMessageBox::warning(this, tr("Folder Not Found"),
            tr("The folder '%1' no longer exists.").arg(path));
        m_settings->recentFolders().removeAll(path);
        m_settings->save();
        updateRecentFoldersMenu();
    }
}

void MainWindow::onCreateFolder()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Create New Folder"),
                                                     m_settings->lastFolderPath());
    if (dir.isEmpty()) return;

    QDir folderDir(dir);
    if (!folderDir.entryList(QDir::AllEntries | QDir::NoDotAndDotDot).isEmpty()) {
        int ret = QMessageBox::question(this, tr("Folder Not Empty"),
            tr("The selected folder is not empty. Open it anyway?"),
            QMessageBox::Yes | QMessageBox::No);
        if (ret != QMessageBox::Yes) return;
    }

    // Create a starter note so the folder isn't completely empty
    QString welcomePath = folderDir.absoluteFilePath("Welcome.md");
    QFile welcomeFile(welcomePath);
    if (welcomeFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&welcomeFile);
        stream << "# Welcome to Durandal\n\n"
               << "This is your new folder. Create notes by right-clicking in the file tree.\n";
        welcomeFile.close();
    }

    openFolder(dir);
}

void MainWindow::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent* event)
{
    const QList<QUrl> urls = event->mimeData()->urls();
    for (const QUrl& url : urls) {
        QString path = url.toLocalFile();
        if (QDir(path).exists()) {
            openFolder(path);
            break;
        }
    }
}

void MainWindow::setWindowTitleForNote(const QString& relativePath)
{
    QFileInfo fi(relativePath);
    QString folderName = m_folder->folderName();
    QString prefix = m_dirty ? "* " : "";
    setWindowTitle(QStringLiteral("%1%2 - %3 - Durandal")
                   .arg(prefix, fi.fileName(), folderName));
}

bool MainWindow::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent* me = static_cast<QMouseEvent*>(event);
        if (me->button() == Qt::LeftButton) {
            QMenuBar* mb = qobject_cast<QMenuBar*>(obj);
            QToolBar* tb = qobject_cast<QToolBar*>(obj);
            if (mb && !mb->actionAt(me->pos())) {
                window()->windowHandle()->startSystemMove();
                return true;
            }
            if (tb && !tb->actionAt(me->pos())) {
                window()->windowHandle()->startSystemMove();
                return true;
            }
        }
    }
    return QMainWindow::eventFilter(obj, event);
}
