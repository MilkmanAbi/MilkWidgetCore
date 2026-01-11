/**
 * MilkWidgetCore - Type definitions and enums
 */

#pragma once

#include <QString>
#include <QColor>
#include <QPoint>
#include <QSize>
#include <QRect>
#include <QFont>
#include <functional>
#include <memory>

namespace Milk {

// ============================================================================
// Enumerations
// ============================================================================

enum class Shape {
    Rectangle,
    RoundedRect,
    Circle,
    Ellipse,
    Square,
    Custom
};

enum class Position {
    TopLeft,
    TopCenter,
    TopRight,
    CenterLeft,
    Center,
    CenterRight,
    BottomLeft,
    BottomCenter,
    BottomRight,
    Manual  // Use x, y coordinates
};

enum class Alignment {
    Left,
    Center,
    Right,
    Top,
    Bottom,
    TopLeft,
    TopRight,
    BottomLeft,
    BottomRight
};

enum class Animation {
    None,
    FadeIn,
    FadeOut,
    SlideIn,
    SlideOut,
    Bounce,
    Pulse,
    Scale,
    Shake,
    Glow
};

enum class Easing {
    Linear,
    InQuad,
    OutQuad,
    InOutQuad,
    InCubic,
    OutCubic,
    InOutCubic,
    InElastic,
    OutElastic,
    InOutElastic,
    InBounce,
    OutBounce,
    InOutBounce
};

enum class GraphType {
    Line,
    Area,
    Bar,
    Sparkline
};

enum class GaugeStyle {
    Arc,
    Circle,
    Linear,
    Semicircle
};

enum class WindowType {
    Normal,
    Desktop,     // Below all windows, like wallpaper
    Dock,        // Reserve space (like taskbar)
    Notification,
    Overlay      // Always on top, click-through
};

enum class BlurMode {
    None,
    Background,  // Blur what's behind the widget
    Glass,       // Glass morphism effect
    Frosted      // Frosted glass
};

enum class BorderStyle {
    None,
    Solid,
    Dashed,
    Dotted,
    Gradient
};

// ============================================================================
// Callback Types
// ============================================================================

using UpdateCallback = std::function<void()>;
using ClickCallback = std::function<void()>;
using HoverCallback = std::function<void(bool)>;
using ValueCallback = std::function<void(double)>;
using TextCallback = std::function<void(const QString&)>;
using AnimationCallback = std::function<void()>;

// ============================================================================
// Data Structures
// ============================================================================

struct Margin {
    int top = 0;
    int right = 0;
    int bottom = 0;
    int left = 0;
    
    Margin() = default;
    Margin(int all) : top(all), right(all), bottom(all), left(all) {}
    Margin(int v, int h) : top(v), right(h), bottom(v), left(h) {}
    Margin(int t, int r, int b, int l) : top(t), right(r), bottom(b), left(l) {}
};

struct Padding : Margin {
    using Margin::Margin;
};

struct Shadow {
    QColor color = QColor(0, 0, 0, 80);
    int blur = 10;
    int offsetX = 0;
    int offsetY = 2;
    int spread = 0;
    bool enabled = false;
};

struct Gradient {
    enum Type { Linear, Radial, Conical } type = Linear;
    QColor start;
    QColor end;
    double angle = 0;  // For linear gradient
    QPointF center;    // For radial/conical
    
    bool isValid() const { return start.isValid() && end.isValid(); }
};

struct Border {
    QColor color = Qt::transparent;
    int width = 0;
    int radius = 0;
    BorderStyle style = BorderStyle::Solid;
    
    bool isVisible() const { return width > 0 && color.alpha() > 0; }
};

struct StyleSheet {
    // Background
    QColor backgroundColor;
    Gradient backgroundGradient;
    QString backgroundImage;
    
    // Text
    QColor textColor;
    QString fontFamily;
    int fontSize = 12;
    bool fontBold = false;
    bool fontItalic = false;
    
    // Geometry
    Border border;
    Shadow shadow;
    Margin margin;
    Padding padding;
    int cornerRadius = 0;
    
    // Effects
    double opacity = 1.0;
    BlurMode blur = BlurMode::None;
    double blurRadius = 10.0;
};

struct SystemInfo {
    double cpuUsage = 0;
    double memoryUsage = 0;
    double diskUsage = 0;
    double temperature = 0;
    QString uptime;
    int processCount = 0;
    double downloadSpeed = 0;
    double uploadSpeed = 0;
    int batteryPercent = 100;
    bool batteryCharging = false;
};

struct WeatherInfo {
    QString condition;
    QString description;
    double temperature = 0;
    double feelsLike = 0;
    int humidity = 0;
    double windSpeed = 0;
    QString windDirection;
    QString icon;
    QString city;
    QDateTime lastUpdate;
};

struct MediaInfo {
    QString title;
    QString artist;
    QString album;
    QString artUrl;
    int duration = 0;
    int position = 0;
    bool playing = false;
    double volume = 1.0;
};

// ============================================================================
// Helper Functions
// ============================================================================

inline QColor parseColor(const QString& str) {
    if (str.startsWith("rgb(")) {
        // Parse rgb(r, g, b)
        QString inner = str.mid(4, str.length() - 5);
        QStringList parts = inner.split(',');
        if (parts.size() >= 3) {
            return QColor(parts[0].trimmed().toInt(),
                         parts[1].trimmed().toInt(),
                         parts[2].trimmed().toInt());
        }
    } else if (str.startsWith("rgba(")) {
        // Parse rgba(r, g, b, a)
        QString inner = str.mid(5, str.length() - 6);
        QStringList parts = inner.split(',');
        if (parts.size() >= 4) {
            return QColor(parts[0].trimmed().toInt(),
                         parts[1].trimmed().toInt(),
                         parts[2].trimmed().toInt(),
                         parts[3].trimmed().toInt());
        }
    }
    return QColor(str);
}

inline QString colorToString(const QColor& c) {
    if (c.alpha() == 255) {
        return c.name();
    }
    return QString("rgba(%1,%2,%3,%4)")
        .arg(c.red()).arg(c.green()).arg(c.blue()).arg(c.alpha());
}

} // namespace Milk
