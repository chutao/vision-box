/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Application Entry Point
 ******************************************************************************/

#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStyle>
#include <QStyleFactory>
#include <QPalette>
#include <QColor>
#include "ui/MainWindow.h"
#include "core/PluginManager.h"

int main(int argc, char* argv[])
{
    // Force X11 backend (xcb) instead of Wayland to avoid dialog rendering issues
    // This ensures proper file dialog display on Wayland sessions
    qputenv("QT_QPA_PLATFORM", "xcb");

    // Create application
    QApplication app(argc, argv);

    // Set application information
    QApplication::setApplicationName("VisionBox");
    QApplication::setApplicationVersion("1.0.0");
    QApplication::setOrganizationName("VisionBox");
    QApplication::setOrganizationDomain("visionbox.com");

    // Set Fusion style (better dark mode support)
    app.setStyle(QStyleFactory::create("Fusion"));

    // Create dark palette
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Disabled, QPalette::WindowText, QColor(127, 127, 127));
    darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Disabled, QPalette::Text, QColor(127, 127, 127));
    darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(127, 127, 127));
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Disabled, QPalette::Highlight, QColor(80, 80, 80));
    darkPalette.setColor(QPalette::HighlightedText, Qt::white);
    darkPalette.setColor(QPalette::Disabled, QPalette::HighlightedText, QColor(127, 127, 127));

    app.setPalette(darkPalette);
    qDebug() << "Applied dark theme palette";

    // Load default style sheet
    QString stylePath;
    QStringList possiblePaths = {
        // Development path
        "../resources/styles/default.qss",
        // Installation path (Linux)
        "/usr/share/VisionBox/styles/default.qss",
        // Installation path (relative to executable)
        QCoreApplication::applicationDirPath() + "/../share/VisionBox/styles/default.qss",
        // Relative path from build directory
        "../resources/styles/default.qss"
    };

    for (const QString& path : possiblePaths)
    {
        QFileInfo fileInfo(path);
        if (fileInfo.exists())
        {
            stylePath = fileInfo.absoluteFilePath();
            break;
        }
    }

    if (!stylePath.isEmpty())
    {
        QFile styleFile(stylePath);
        if (styleFile.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QString styleSheet = QString::fromUtf8(styleFile.readAll());
            app.setStyleSheet(styleSheet);
            qDebug() << "Loaded style sheet from:" << stylePath;
        }
        else
        {
            qWarning() << "Failed to open style file:" << stylePath;
        }
    }
    else
    {
        qWarning() << "Style file not found in any of the expected locations";
        qDebug() << "Searched paths:" << possiblePaths;
    }

    // Parse command line arguments
    QCommandLineParser parser;
    parser.setApplicationDescription("VisionBox - Computer Vision Research Framework");
    parser.addHelpOption();
    parser.addVersionOption();

    // Option to specify plugin directories
    QCommandLineOption pluginDirOption(QStringList() << "p" << "plugin-dir",
        "Load plugins from <directory>.",
        "directory");
    parser.addOption(pluginDirOption);

    // Option to list all plugins
    QCommandLineOption listPluginsOption(QStringList() << "l" << "list-plugins",
        "List all loaded plugins and exit.");
    parser.addOption(listPluginsOption);

    // Option to disable plugin auto-loading
    QCommandLineOption noAutoLoadOption("no-auto-load",
        "Disable automatic plugin loading from default directories.");
    parser.addOption(noAutoLoadOption);

    parser.process(app);

    // Get plugin manager instance
    VisionBox::PluginManager* pluginManager = VisionBox::PluginManager::instance();

    // Add custom plugin directories from command line
    QStringList pluginDirs = parser.values(pluginDirOption);
    for (const QString& dir : pluginDirs)
    {
        if (QDir(dir).exists())
        {
            pluginManager->addPluginDirectory(dir);
            qDebug() << "Added plugin directory:" << dir;
        }
        else
        {
            qWarning() << "Plugin directory does not exist:" << dir;
        }
    }

    // Load plugins from default directories (unless disabled)
    if (!parser.isSet(noAutoLoadOption))
    {
        qDebug() << "Loading plugins from default directories...";
        int totalLoaded = 0;
        for (const QString& pluginDir : pluginManager->getPluginDirectories())
        {
            int loaded = pluginManager->loadPluginsFromDirectory(pluginDir);
            totalLoaded += loaded;
            if (loaded > 0)
            {
                qDebug() << "  Loaded" << loaded << "plugins from:" << pluginDir;
            }
        }
        qDebug() << "Total plugins loaded:" << totalLoaded;
    }

    // Load plugins from custom directories
    for (const QString& dir : pluginDirs)
    {
        int loaded = pluginManager->loadPluginsFromDirectory(dir);
        qDebug() << "Loaded" << loaded << "plugins from:" << dir;
    }

    // List plugins and exit if requested
    if (parser.isSet(listPluginsOption))
    {
        QVector<VisionBox::PluginInfo> plugins = pluginManager->getLoadedPlugins();

        qDebug() << "\n=== Loaded Plugins ===";
        if (plugins.isEmpty())
        {
            qDebug() << "No plugins loaded.";
        }
        else
        {
            for (const auto& plugin : plugins)
            {
                qDebug() << "\nPlugin:" << plugin.name;
                qDebug() << "  ID:" << plugin.id;
                qDebug() << "  Version:" << plugin.version;
                if (!plugin.description.isEmpty())
                {
                    qDebug() << "  Description:" << plugin.description;
                }
                if (!plugin.author.isEmpty())
                {
                    qDebug() << "  Author:" << plugin.author;
                }
                if (!plugin.categories.isEmpty())
                {
                    qDebug() << "  Categories:" << plugin.categories.join(", ");
                }
            }
        }
        qDebug() << "\nTotal:" << plugins.size() << "plugin(s)\n";

        return 0;
    }

    // Create and show main window
    qDebug() << "\n=== Starting VisionBox ===";
    qDebug() << "Qt version:" << QT_VERSION_STR;

    VisionBox::MainWindow window;
    window.show();

    // Display plugin status on startup
    QVector<VisionBox::PluginInfo> plugins = pluginManager->getLoadedPlugins();
    qDebug() << "Loaded plugins:" << plugins.size();
    for (const auto& plugin : plugins)
    {
        qDebug() << "  -" << plugin.name << "v" << plugin.version;
    }
    qDebug() << "======================\n";

    // Run application
    return app.exec();
}
