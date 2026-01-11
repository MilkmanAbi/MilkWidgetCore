/**
 * MilkWidgetCore - Utilities
 * 
 * Color utilities, animation engine, logging, etc.
 */

#pragma once

#include <QObject>
#include <QColor>
#include <QString>
#include <QEasingCurve>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>
#include <functional>
#include <memory>

#include "Types.h"

namespace Milk {

// ============================================================================
// COLOR UTILITIES
// ============================================================================
namespace Color {

/**
 * Parse color from string
 * Supports: #RGB, #RRGGBB, #RRGGBBAA, rgb(), rgba(), hsl(), hsla(), named colors
 */
QColor parse(const QString& str);

/**
 * Convert color to string
 */
QString toString(const QColor& color, bool includeAlpha = true);
QString toHex(const QColor& color, bool includeAlpha = false);
QString toRgb(const QColor& color);
QString toRgba(const QColor& color);
QString toHsl(const QColor& color);

/**
 * Color manipulation
 */
QColor lighten(const QColor& color, double amount = 0.1);
QColor darken(const QColor& color, double amount = 0.1);
QColor saturate(const QColor& color, double amount = 0.1);
QColor desaturate(const QColor& color, double amount = 0.1);
QColor adjustAlpha(const QColor& color, double alpha);
QColor withAlpha(const QColor& color, int alpha);

/**
 * Color blending
 */
QColor mix(const QColor& a, const QColor& b, double ratio = 0.5);
QColor overlay(const QColor& base, const QColor& blend);

/**
 * Color analysis
 */
double luminance(const QColor& color);
double contrast(const QColor& a, const QColor& b);
bool isDark(const QColor& color);
bool isLight(const QColor& color);
QColor contrastingText(const QColor& background);

/**
 * Color generation
 */
QColor random();
QColor randomPastel();
QColor randomVibrant();
QList<QColor> palette(const QColor& base, int count);
QList<QColor> gradient(const QColor& start, const QColor& end, int steps);

/**
 * Predefined colors
 */
extern const QColor Transparent;
extern const QColor White;
extern const QColor Black;
extern const QColor Red;
extern const QColor Green;
extern const QColor Blue;
extern const QColor Yellow;
extern const QColor Cyan;
extern const QColor Magenta;
extern const QColor Orange;
extern const QColor Purple;
extern const QColor Pink;
extern const QColor Gray;
extern const QColor DarkGray;
extern const QColor LightGray;

} // namespace Color

// ============================================================================
// ANIMATION ENGINE
// ============================================================================
class AnimationEngine : public QObject {
    Q_OBJECT
    
public:
    static AnimationEngine* instance();
    
    /**
     * Animate a property
     */
    QPropertyAnimation* animate(QObject* target, const QByteArray& property,
                                const QVariant& endValue, int duration = 300,
                                Easing easing = Easing::OutCubic);
    
    /**
     * Animate with start and end values
     */
    QPropertyAnimation* animate(QObject* target, const QByteArray& property,
                                const QVariant& startValue, const QVariant& endValue,
                                int duration = 300, Easing easing = Easing::OutCubic);
    
    /**
     * Create animation group (parallel)
     */
    QParallelAnimationGroup* parallel();
    
    /**
     * Create animation group (sequential)
     */
    QSequentialAnimationGroup* sequential();
    
    /**
     * Stop all animations for target
     */
    void stopAll(QObject* target);
    
    /**
     * Pause all animations for target
     */
    void pauseAll(QObject* target);
    
    /**
     * Resume all animations for target
     */
    void resumeAll(QObject* target);
    
    /**
     * Convert Easing enum to Qt easing curve
     */
    static QEasingCurve::Type toQtEasing(Easing easing);
    
    /**
     * Preset animations
     */
    void fadeIn(QWidget* widget, int duration = 300);
    void fadeOut(QWidget* widget, int duration = 300);
    void slideIn(QWidget* widget, Position from, int duration = 300);
    void slideOut(QWidget* widget, Position to, int duration = 300);
    void bounce(QWidget* widget, int duration = 500);
    void pulse(QWidget* widget, int duration = 1000, int count = 1);
    void shake(QWidget* widget, int duration = 500, int intensity = 10);
    void scale(QWidget* widget, double from, double to, int duration = 300);
    
signals:
    void animationStarted(QObject* target, const QString& property);
    void animationFinished(QObject* target, const QString& property);
    
private:
    AnimationEngine();
    
private:
    static AnimationEngine* s_instance;
    QMap<QObject*, QList<QAbstractAnimation*>> m_animations;
};

// Global animation accessor
AnimationEngine* anim();

// ============================================================================
// LOGGER
// ============================================================================
class Logger : public QObject {
    Q_OBJECT
    
public:
    enum Level {
        Debug,
        Info,
        Warning,
        Error,
        Fatal
    };
    
    static Logger* instance();
    
    /**
     * Log messages
     */
    void debug(const QString& message);
    void info(const QString& message);
    void warning(const QString& message);
    void error(const QString& message);
    void fatal(const QString& message);
    
    /**
     * Log with category
     */
    void log(Level level, const QString& category, const QString& message);
    
    /**
     * Configuration
     */
    void setLogLevel(Level level);
    Level logLevel() const { return m_level; }
    
    void setLogToFile(bool enabled, const QString& path = "");
    void setLogToConsole(bool enabled);
    void setColorOutput(bool enabled);
    
    /**
     * Format
     */
    void setFormat(const QString& format);
    
signals:
    void logged(int level, const QString& category, const QString& message);
    
private:
    Logger();
    QString formatMessage(Level level, const QString& category, const QString& message);
    
private:
    static Logger* s_instance;
    
    Level m_level = Info;
    bool m_logToFile = false;
    bool m_logToConsole = true;
    bool m_colorOutput = true;
    QString m_logPath;
    QString m_format = "[%time%] [%level%] %category%: %message%";
};

// Global logger accessor
Logger* log();

// Convenience macros
#define MILK_DEBUG(msg) Milk::log()->debug(msg)
#define MILK_INFO(msg) Milk::log()->info(msg)
#define MILK_WARN(msg) Milk::log()->warning(msg)
#define MILK_ERROR(msg) Milk::log()->error(msg)

// ============================================================================
// FILE UTILITIES
// ============================================================================
namespace File {

/**
 * Read file contents
 */
QString readText(const QString& path);
QByteArray readBytes(const QString& path);

/**
 * Write file contents
 */
bool writeText(const QString& path, const QString& content);
bool writeBytes(const QString& path, const QByteArray& data);

/**
 * Check file existence
 */
bool exists(const QString& path);
bool isFile(const QString& path);
bool isDirectory(const QString& path);

/**
 * Path manipulation
 */
QString baseName(const QString& path);
QString dirName(const QString& path);
QString extension(const QString& path);
QString join(const QString& a, const QString& b);
QString absolute(const QString& path);
QString relative(const QString& path, const QString& base);

/**
 * Create directories
 */
bool mkdirs(const QString& path);

/**
 * List directory contents
 */
QStringList listFiles(const QString& path, const QStringList& filters = {});
QStringList listDirs(const QString& path);

/**
 * Config paths
 */
QString configDir();
QString dataDir();
QString cacheDir();
QString homeDir();
QString tempDir();

} // namespace File

// ============================================================================
// STRING UTILITIES
// ============================================================================
namespace String {

/**
 * Format bytes as human readable
 */
QString formatBytes(qint64 bytes);

/**
 * Format duration
 */
QString formatDuration(int seconds);
QString formatDurationMs(int milliseconds);

/**
 * Format percentage
 */
QString formatPercent(double value, int decimals = 0);

/**
 * Format temperature
 */
QString formatTemperature(double celsius, bool fahrenheit = false);

/**
 * Case conversion
 */
QString toCamelCase(const QString& str);
QString toSnakeCase(const QString& str);
QString toKebabCase(const QString& str);
QString toTitleCase(const QString& str);

/**
 * Truncation
 */
QString truncate(const QString& str, int maxLength, const QString& suffix = "...");
QString ellipsis(const QString& str, int maxLength);

/**
 * Parsing
 */
int toInt(const QString& str, int defaultValue = 0);
double toDouble(const QString& str, double defaultValue = 0.0);
bool toBool(const QString& str, bool defaultValue = false);

} // namespace String

// ============================================================================
// TIMER UTILITIES
// ============================================================================

/**
 * Create a timer with callback
 */
QTimer* createTimer(int interval, std::function<void()> callback);

/**
 * Single-shot timer
 */
void delay(int ms, std::function<void()> callback);

/**
 * Debounce a function call
 */
void debounce(QObject* context, int ms, std::function<void()> callback);

/**
 * Throttle a function call
 */
void throttle(QObject* context, int ms, std::function<void()> callback);

// ============================================================================
// SCREEN UTILITIES
// ============================================================================
namespace Screen {

/**
 * Get screen geometry
 */
QSize size();
QRect geometry();
QRect availableGeometry();
QPoint center();

/**
 * Get screen at point
 */
int screenAt(const QPoint& point);
int screenCount();

/**
 * DPI
 */
double dpi();
double scaleFactor();

/**
 * Position calculation
 */
QPoint calculatePosition(Position pos, const QSize& widgetSize, int margin = 50);

} // namespace Screen

} // namespace Milk
