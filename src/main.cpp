#include <QApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QIcon>
#include "MainWindow.h"
#include "AppSettings.h"
#include "Version.h"

#ifdef Q_OS_WIN
#include <QtWebEngineCore/QWebEngineUrlScheme>
#include <QtWebEngineQuick/qtwebenginequickglobal.h>
#endif

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Durandal");
    app.setApplicationVersion(DURANDAL_VERSION);
    app.setOrganizationName("Durandal");
    app.setOrganizationDomain("durandal.app");
    app.setWindowIcon(QIcon(":/icons/icon.svg"));

    QCommandLineParser parser;
    parser.setApplicationDescription("A cross-platform note-taking application");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("folder", "Path to the folder to open on startup");
    parser.process(app);

    AppSettings settings;
    settings.load();

    MainWindow window;
    window.show();

    QString folderPath;
    if (!parser.positionalArguments().isEmpty()) {
        folderPath = parser.positionalArguments().first();
    } else if (!settings.lastFolderPath().isEmpty()) {
        folderPath = settings.lastFolderPath();
    }

    if (!folderPath.isEmpty() && QDir(folderPath).exists()) {
        window.openFolderPath(folderPath);
    }

    return app.exec();
}
