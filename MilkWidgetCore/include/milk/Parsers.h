/**
 * MilkWidgetCore - Parsers
 * 
 * XML and CSS parsing for widget configuration
 */

#pragma once

#include <QObject>
#include <QString>
#include <QDomDocument>
#include <QMap>
#include <QVariant>
#include <memory>

#include "Types.h"

namespace Milk {

class Widget;

// ============================================================================
// XML PARSER
// ============================================================================
class XMLParser : public QObject {
    Q_OBJECT
    
public:
    explicit XMLParser(QObject* parent = nullptr);
    virtual ~XMLParser() = default;
    
    /**
     * Parse XML file and return list of widgets
     */
    QList<Widget*> parseFile(const QString& path);
    
    /**
     * Parse XML string and return list of widgets
     */
    QList<Widget*> parseString(const QString& xml);
    
    /**
     * Convert widget to XML string
     */
    QString toXml(Widget* widget);
    
    /**
     * Save widget to XML file
     */
    bool saveToFile(Widget* widget, const QString& path);
    
    /**
     * Get last error message
     */
    QString lastError() const { return m_lastError; }
    
    /**
     * Check if last operation was successful
     */
    bool hasError() const { return !m_lastError.isEmpty(); }
    
signals:
    void parseError(const QString& error, int line, int column);
    void widgetCreated(Widget* widget);
    
private:
    Widget* parseWidget(const QDomElement& elem);
    void parseWidgetProperties(Widget* widget, const QDomElement& elem);
    void parseChildren(Widget* parent, const QDomElement& elem);
    QWidget* parseChildElement(const QDomElement& elem, Widget* parent);
    
    // Attribute parsing helpers
    QColor parseColor(const QString& value);
    Position parsePosition(const QString& value);
    Shape parseShape(const QString& value);
    Alignment parseAlignment(const QString& value);
    
private:
    QString m_lastError;
    QString m_basePath;  // For relative paths
};

// ============================================================================
// CSS PARSER
// ============================================================================
class CSSParser : public QObject {
    Q_OBJECT
    
public:
    explicit CSSParser(QObject* parent = nullptr);
    virtual ~CSSParser() = default;
    
    /**
     * Parse CSS file
     */
    bool parseFile(const QString& path);
    
    /**
     * Parse CSS string
     */
    bool parseString(const QString& css);
    
    /**
     * Get style for a class name
     */
    StyleSheet getStyle(const QString& className);
    
    /**
     * Get style for element type
     */
    StyleSheet getTypeStyle(const QString& typeName);
    
    /**
     * Apply styles to widget by class name
     */
    void applyStyle(Widget* widget, const QString& className);
    
    /**
     * Get all class names
     */
    QStringList classNames() const;
    
    /**
     * Get CSS string for a style
     */
    QString toCSS(const StyleSheet& style);
    
    /**
     * Merge two stylesheets (b overrides a)
     */
    static StyleSheet merge(const StyleSheet& a, const StyleSheet& b);
    
    /**
     * Get last error
     */
    QString lastError() const { return m_lastError; }
    
private:
    void parseRule(const QString& selector, const QString& body);
    void parseProperty(StyleSheet& style, const QString& property, const QString& value);
    
    // Value parsing helpers
    QColor parseColor(const QString& value);
    int parsePixels(const QString& value);
    Margin parseMargin(const QString& value);
    Border parseBorder(const QString& value);
    Shadow parseShadow(const QString& value);
    Gradient parseGradient(const QString& value);
    
private:
    QMap<QString, StyleSheet> m_styles;
    QString m_lastError;
};

// ============================================================================
// WIDGET FACTORY
// ============================================================================
class WidgetFactory : public QObject {
    Q_OBJECT
    
public:
    static WidgetFactory* instance();
    
    /**
     * Register a custom widget type
     */
    template<typename T>
    void registerWidget(const QString& typeName) {
        m_creators[typeName] = []() -> QWidget* { return new T(); };
    }
    
    /**
     * Create widget by type name
     */
    QWidget* createWidget(const QString& typeName);
    
    /**
     * Check if type is registered
     */
    bool hasType(const QString& typeName) const;
    
    /**
     * Get all registered type names
     */
    QStringList typeNames() const;
    
    /**
     * Create widget from XML element
     */
    QWidget* createFromXml(const QDomElement& elem, Widget* parent = nullptr);
    
private:
    WidgetFactory();
    void registerBuiltinTypes();
    
private:
    static WidgetFactory* s_instance;
    
    using WidgetCreator = std::function<QWidget*()>;
    QMap<QString, WidgetCreator> m_creators;
};

// ============================================================================
// THEME MANAGER
// ============================================================================
class ThemeManager : public QObject {
    Q_OBJECT
    
public:
    explicit ThemeManager(QObject* parent = nullptr);
    virtual ~ThemeManager() = default;
    
    /**
     * Load theme from directory
     * Theme directory should contain:
     *   - theme.xml or theme.css (main style)
     *   - widgets/ (widget definitions)
     *   - assets/ (images, fonts, etc.)
     */
    bool loadTheme(const QString& themePath);
    
    /**
     * Get current theme name
     */
    QString currentTheme() const { return m_currentTheme; }
    
    /**
     * Get available themes
     */
    QStringList availableThemes() const;
    
    /**
     * Set theme search paths
     */
    void setThemePaths(const QStringList& paths);
    void addThemePath(const QString& path);
    
    /**
     * Get style for class
     */
    StyleSheet getStyle(const QString& className);
    
    /**
     * Get asset path
     */
    QString assetPath(const QString& name) const;
    
    /**
     * Reload current theme
     */
    void reload();
    
signals:
    void themeChanged(const QString& themeName);
    void themeReloaded();
    
private:
    void scanThemes();
    
private:
    QString m_currentTheme;
    QString m_themePath;
    QStringList m_searchPaths;
    QMap<QString, QString> m_themes;  // name -> path
    
    std::unique_ptr<CSSParser> m_cssParser;
    std::unique_ptr<XMLParser> m_xmlParser;
};

// ============================================================================
// CONFIG WATCHER
// ============================================================================
class ConfigWatcher : public QObject {
    Q_OBJECT
    
public:
    explicit ConfigWatcher(QObject* parent = nullptr);
    virtual ~ConfigWatcher();
    
    /**
     * Watch a file or directory for changes
     */
    void watch(const QString& path);
    
    /**
     * Stop watching a path
     */
    void unwatch(const QString& path);
    
    /**
     * Stop watching all
     */
    void clear();
    
    /**
     * Get watched paths
     */
    QStringList watchedPaths() const;
    
    /**
     * Enable/disable watching
     */
    void setEnabled(bool enabled);
    bool isEnabled() const { return m_enabled; }
    
signals:
    void fileChanged(const QString& path);
    void directoryChanged(const QString& path);
    
private:
    class Private;
    std::unique_ptr<Private> d;
    bool m_enabled = true;
};

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

/**
 * Load widgets from XML file
 */
QList<Widget*> loadXml(const QString& path);

/**
 * Load styles from CSS file
 */
bool loadCSS(const QString& path);

/**
 * Get global CSS parser
 */
CSSParser* globalCSS();

/**
 * Get global XML parser
 */
XMLParser* globalXML();

} // namespace Milk
