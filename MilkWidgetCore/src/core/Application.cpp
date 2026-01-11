/**
 * MilkWidgetCore - Application Implementation
 */

#include "milk/Application.h"
#include "milk/Widget.h"
#include "milk/Parsers.h"
#include "milk/APIs.h"
#include "milk/Utils.h"

#include <QScreen>
#include <QDir>
#include <QStandardPaths>

namespace Milk {

Application* Application::s_instance = nullptr;

Application::Application(int& argc, char** argv)
    : QApplication(argc, argv)
{
    s_instance = this;
    
    setApplicationName("MilkWidget");
    setApplicationVersion(MILK_VERSION_STRING);
    setOrganizationName("MilkWidget");
    
    // Set default paths
    m_configDir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/milkwidget";
    m_themeDir = m_configDir + "/themes";
    
    // Ensure directories exist
    QDir().mkpath(m_configDir);
    QDir().mkpath(m_themeDir);
    
    // Initialize subsystems
    initializeSubsystems();
    
    // Connect quit signal
    connect(this, &QApplication::aboutToQuit, this, &Application::onAboutToQuit);
}

Application::~Application() {
    cleanupWidgets();
    cleanupAPIs();
    s_instance = nullptr;
}

Application* Application::instance() {
    return s_instance;
}

void Application::initializeSubsystems() {
    // Create theme manager
    m_themeManager = std::make_unique<ThemeManager>(this);
    m_themeManager->addThemePath(m_themeDir);
    m_themeManager->addThemePath("/usr/share/milkwidget/themes");
    m_themeManager->addThemePath("/usr/local/share/milkwidget/themes");
    
    // Create config watcher
    m_configWatcher = std::make_unique<ConfigWatcher>(this);
    connect(m_configWatcher.get(), &ConfigWatcher::fileChanged,
            this, &Application::onConfigChanged);
}

// ============================================================================
// WIDGET MANAGEMENT
// ============================================================================

QList<Widget*> Application::loadWidgets(const QString& xmlPath) {
    XMLParser parser;
    QList<Widget*> widgets = parser.parseFile(xmlPath);
    
    for (Widget* w : widgets) {
        registerWidget(w);
    }
    
    // Watch for changes
    if (m_autoReload) {
        m_configWatcher->watch(xmlPath);
    }
    
    return widgets;
}

bool Application::loadTheme(const QString& themePath) {
    return m_themeManager->loadTheme(themePath);
}

QList<Widget*> Application::loadDirectory(const QString& dirPath) {
    QList<Widget*> widgets;
    QDir dir(dirPath);
    
    QStringList filters;
    filters << "*.xml" << "*.milk";
    
    for (const QString& file : dir.entryList(filters, QDir::Files)) {
        QList<Widget*> loaded = loadWidgets(dir.absoluteFilePath(file));
        widgets.append(loaded);
    }
    
    return widgets;
}

void Application::registerWidget(Widget* widget) {
    if (widget && !m_widgets.contains(widget)) {
        m_widgets.append(widget);
        
        connect(widget, &QObject::destroyed, [this, widget]() {
            m_widgets.removeAll(widget);
        });
        
        emit widgetAdded(widget);
    }
}

void Application::unregisterWidget(Widget* widget) {
    if (m_widgets.removeAll(widget) > 0) {
        emit widgetRemoved(widget);
    }
}

void Application::showAll() {
    for (Widget* w : m_widgets) {
        w->show();
    }
}

void Application::hideAll() {
    for (Widget* w : m_widgets) {
        w->hide();
    }
}

void Application::toggleAll() {
    for (Widget* w : m_widgets) {
        w->toggle();
    }
}

void Application::cleanupWidgets() {
    for (Widget* w : m_widgets) {
        delete w;
    }
    m_widgets.clear();
}

// ============================================================================
// SYSTEM TRAY
// ============================================================================

void Application::enableTrayIcon(bool enabled) {
    m_trayEnabled = enabled;
    
    if (enabled && !m_trayIcon) {
        setupTray();
    } else if (!enabled && m_trayIcon) {
        m_trayIcon->hide();
        m_trayIcon.reset();
        m_trayMenu.reset();
    }
}

void Application::setupTray() {
    m_trayIcon = std::make_unique<QSystemTrayIcon>(this);
    m_trayMenu = std::make_unique<QMenu>();
    
    // Default icon
    m_trayIcon->setIcon(QIcon::fromTheme("preferences-desktop-display"));
    m_trayIcon->setToolTip("MilkWidget");
    
    // Default menu
    QAction* showAction = m_trayMenu->addAction("Show All");
    connect(showAction, &QAction::triggered, this, &Application::showAll);
    
    QAction* hideAction = m_trayMenu->addAction("Hide All");
    connect(hideAction, &QAction::triggered, this, &Application::hideAll);
    
    m_trayMenu->addSeparator();
    
    QAction* quitAction = m_trayMenu->addAction("Quit");
    connect(quitAction, &QAction::triggered, this, &QApplication::quit);
    
    m_trayIcon->setContextMenu(m_trayMenu.get());
    
    connect(m_trayIcon.get(), &QSystemTrayIcon::activated,
            this, &Application::onTrayActivated);
    
    m_trayIcon->show();
}

void Application::setTrayIcon(const QString& iconPath) {
    if (m_trayIcon) {
        m_trayIcon->setIcon(QIcon(iconPath));
    }
}

void Application::setTrayIcon(const QIcon& icon) {
    if (m_trayIcon) {
        m_trayIcon->setIcon(icon);
    }
}

void Application::setTrayTooltip(const QString& tooltip) {
    if (m_trayIcon) {
        m_trayIcon->setToolTip(tooltip);
    }
}

void Application::addTrayAction(const QString& text, std::function<void()> callback) {
    if (m_trayMenu) {
        QAction* action = m_trayMenu->addAction(text);
        connect(action, &QAction::triggered, callback);
    }
}

void Application::addTraySeparator() {
    if (m_trayMenu) {
        m_trayMenu->addSeparator();
    }
}

void Application::onTrayActivated(QSystemTrayIcon::ActivationReason reason) {
    if (reason == QSystemTrayIcon::Trigger ||
        reason == QSystemTrayIcon::DoubleClick) {
        toggleAll();
        emit trayActivated();
    }
}

// ============================================================================
// CONFIGURATION
// ============================================================================

void Application::setConfigDir(const QString& path) {
    m_configDir = path;
    QDir().mkpath(m_configDir);
}

void Application::setThemeDir(const QString& path) {
    m_themeDir = path;
    QDir().mkpath(m_themeDir);
    m_themeManager->addThemePath(m_themeDir);
}

void Application::setAutoReload(bool enabled) {
    m_autoReload = enabled;
    m_configWatcher->setEnabled(enabled);
}

void Application::setGlobalUpdateInterval(int ms) {
    m_globalUpdateInterval = ms;
    
    for (Widget* w : m_widgets) {
        w->setUpdateInterval(ms);
    }
}

void Application::onConfigChanged(const QString& path) {
    log()->info(QString("Config file changed: %1").arg(path));
    
    // Reload widgets from changed file
    // This is a simple implementation - could be smarter
    emit configReloaded();
}

void Application::onAboutToQuit() {
    cleanupWidgets();
    cleanupAPIs();
}

// ============================================================================
// DESKTOP INTEGRATION
// ============================================================================

bool Application::isWayland() {
    return platformName() == "wayland";
}

bool Application::isX11() {
    return platformName() == "xcb";
}

QString Application::desktopEnvironment() {
    return qEnvironmentVariable("XDG_CURRENT_DESKTOP", "Unknown");
}

QSize Application::screenSize() {
    QScreen* screen = primaryScreen();
    return screen ? screen->size() : QSize(1920, 1080);
}

QPoint Application::screenCenter() {
    QSize size = screenSize();
    return QPoint(size.width() / 2, size.height() / 2);
}

QRect Application::availableGeometry() {
    QScreen* screen = primaryScreen();
    return screen ? screen->availableGeometry() : QRect(0, 0, 1920, 1080);
}

} // namespace Milk
