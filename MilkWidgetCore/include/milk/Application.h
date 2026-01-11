/**
 * MilkWidgetCore - Application Class
 * 
 * Handles initialization, event loop, and global configuration
 */

#pragma once

#include <QApplication>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QList>
#include <memory>

#include "Types.h"

namespace Milk {

class Widget;
class ThemeManager;
class ConfigWatcher;

class Application : public QApplication {
    Q_OBJECT
    
public:
    Application(int& argc, char** argv);
    virtual ~Application();
    
    // Singleton access
    static Application* instance();
    
    // ========================================================================
    // Widget Management
    // ========================================================================
    
    /**
     * Load widgets from an XML file
     * Returns list of created widgets
     */
    QList<Widget*> loadWidgets(const QString& xmlPath);
    
    /**
     * Load a theme directory
     * Theme directories contain widget XML files and resources
     */
    bool loadTheme(const QString& themePath);
    
    /**
     * Load all widgets from a directory
     */
    QList<Widget*> loadDirectory(const QString& dirPath);
    
    /**
     * Register a widget for management
     */
    void registerWidget(Widget* widget);
    
    /**
     * Unregister a widget
     */
    void unregisterWidget(Widget* widget);
    
    /**
     * Get all managed widgets
     */
    QList<Widget*> widgets() const { return m_widgets; }
    
    /**
     * Show all widgets
     */
    void showAll();
    
    /**
     * Hide all widgets
     */
    void hideAll();
    
    /**
     * Toggle visibility of all widgets
     */
    void toggleAll();
    
    // ========================================================================
    // System Tray
    // ========================================================================
    
    /**
     * Enable system tray icon
     */
    void enableTrayIcon(bool enabled = true);
    
    /**
     * Set tray icon
     */
    void setTrayIcon(const QString& iconPath);
    void setTrayIcon(const QIcon& icon);
    
    /**
     * Set tray tooltip
     */
    void setTrayTooltip(const QString& tooltip);
    
    /**
     * Add action to tray menu
     */
    void addTrayAction(const QString& text, std::function<void()> callback);
    
    /**
     * Add separator to tray menu
     */
    void addTraySeparator();
    
    // ========================================================================
    // Configuration
    // ========================================================================
    
    /**
     * Set global configuration directory
     */
    void setConfigDir(const QString& path);
    QString configDir() const { return m_configDir; }
    
    /**
     * Set theme directory
     */
    void setThemeDir(const QString& path);
    QString themeDir() const { return m_themeDir; }
    
    /**
     * Enable auto-reload when config files change
     */
    void setAutoReload(bool enabled);
    bool autoReload() const { return m_autoReload; }
    
    /**
     * Set global update interval for all widgets (ms)
     */
    void setGlobalUpdateInterval(int ms);
    int globalUpdateInterval() const { return m_globalUpdateInterval; }
    
    // ========================================================================
    // Desktop Integration
    // ========================================================================
    
    /**
     * Check if running on Wayland
     */
    static bool isWayland();
    
    /**
     * Check if running on X11
     */
    static bool isX11();
    
    /**
     * Get desktop environment name
     */
    static QString desktopEnvironment();
    
    /**
     * Get screen size
     */
    static QSize screenSize();
    
    /**
     * Get screen center
     */
    static QPoint screenCenter();
    
    /**
     * Get available geometry (minus panels/taskbars)
     */
    static QRect availableGeometry();
    
    // ========================================================================
    // Theme Manager
    // ========================================================================
    
    ThemeManager* themeManager() { return m_themeManager.get(); }
    
signals:
    void widgetAdded(Widget* widget);
    void widgetRemoved(Widget* widget);
    void themeChanged(const QString& themeName);
    void configReloaded();
    void trayActivated();
    
private slots:
    void onTrayActivated(QSystemTrayIcon::ActivationReason reason);
    void onConfigChanged(const QString& path);
    void onAboutToQuit();
    
private:
    void setupTray();
    void cleanupWidgets();
    void initializeSubsystems();
    
private:
    // Widgets
    QList<Widget*> m_widgets;
    
    // System tray
    std::unique_ptr<QSystemTrayIcon> m_trayIcon;
    std::unique_ptr<QMenu> m_trayMenu;
    bool m_trayEnabled = false;
    
    // Configuration
    QString m_configDir;
    QString m_themeDir;
    bool m_autoReload = true;
    int m_globalUpdateInterval = 1000;
    
    // Managers
    std::unique_ptr<ThemeManager> m_themeManager;
    std::unique_ptr<ConfigWatcher> m_configWatcher;
    
    // Singleton
    static Application* s_instance;
};

} // namespace Milk
