/**
 * MilkWidgetCore - CLI Widget Runner
 * 
 * Simple command-line tool to load and display widgets from XML files
 */

#include <milk/MilkWidget.h>
#include <QCommandLineParser>
#include <QFileInfo>
#include <iostream>

using namespace Milk;

void printUsage() {
    std::cout << "MilkWidget - Desktop Widget Engine\n"
              << "Usage: milkwidget [options] [config.xml ...]\n\n"
              << "Options:\n"
              << "  -h, --help           Show this help\n"
              << "  -v, --version        Show version\n"
              << "  -d, --daemon         Run as daemon (background)\n"
              << "  -t, --theme <name>   Load theme\n"
              << "  -c, --config <dir>   Config directory\n"
              << "  --list-themes        List available themes\n"
              << "\nExamples:\n"
              << "  milkwidget system_monitor.xml\n"
              << "  milkwidget -t dark ~/.config/milkwidget/*.xml\n"
              << "  milkwidget --daemon\n";
}

int main(int argc, char *argv[])
{
    Application app(argc, argv);
    app.setApplicationName("MilkWidget");
    app.setApplicationVersion(MILK_VERSION_STRING);
    
    QCommandLineParser parser;
    parser.setApplicationDescription("Desktop Widget Engine");
    parser.addHelpOption();
    parser.addVersionOption();
    
    QCommandLineOption daemonOpt({"d", "daemon"}, "Run as daemon");
    parser.addOption(daemonOpt);
    
    QCommandLineOption themeOpt({"t", "theme"}, "Load theme", "name");
    parser.addOption(themeOpt);
    
    QCommandLineOption configOpt({"c", "config"}, "Config directory", "dir");
    parser.addOption(configOpt);
    
    QCommandLineOption listThemesOpt("list-themes", "List available themes");
    parser.addOption(listThemesOpt);
    
    parser.addPositionalArgument("files", "Widget XML files to load", "[files...]");
    
    parser.process(app);
    
    // List themes
    if (parser.isSet(listThemesOpt)) {
        std::cout << "Available themes:\n";
        for (const QString& theme : app.themeManager()->availableThemes()) {
            std::cout << "  " << theme.toStdString() << "\n";
        }
        return 0;
    }
    
    // Set config directory
    if (parser.isSet(configOpt)) {
        app.setConfigDir(parser.value(configOpt));
    }
    
    // Load theme
    if (parser.isSet(themeOpt)) {
        app.loadTheme(parser.value(themeOpt));
    }
    
    // Load widget files
    QStringList files = parser.positionalArguments();
    
    if (files.isEmpty()) {
        // Load from default config directory
        QString configDir = app.configDir();
        QDir dir(configDir);
        if (dir.exists()) {
            files = dir.entryList({"*.xml"}, QDir::Files);
            for (QString& f : files) {
                f = dir.filePath(f);
            }
        }
    }
    
    if (files.isEmpty()) {
        log()->info("No widget files specified. Use --help for usage.");
        printUsage();
        return 1;
    }
    
    // Load each widget file
    int loaded = 0;
    for (const QString& file : files) {
        if (QFileInfo::exists(file)) {
            QList<Widget*> widgets = app.loadWidgets(file);
            loaded += widgets.size();
            log()->info(QString("Loaded %1 widgets from %2").arg(widgets.size()).arg(file));
        } else {
            log()->warning(QString("File not found: %1").arg(file));
        }
    }
    
    if (loaded == 0) {
        log()->error("No widgets loaded.");
        return 1;
    }
    
    log()->info(QString("Total %1 widgets loaded.").arg(loaded));
    
    // Enable system tray
    app.enableTrayIcon(true);
    app.setTrayTooltip(QString("MilkWidget (%1 widgets)").arg(loaded));
    
    // Show all widgets
    app.showAll();
    
    // Run
    return app.exec();
}
