/**
 * MilkWidgetCore - All Widget Types
 */

#pragma once

#include <QLabel>
#include <QWidget>
#include <QPainter>
#include <QTimer>
#include <QDateTime>
#include <QImage>
#include <QPushButton>

#include "Types.h"
#include "Widget.h"

namespace Milk {

// Forward declarations
class Widget;

// ============================================================================
// TEXT WIDGET
// ============================================================================
class Text : public QLabel {
    Q_OBJECT
    
public:
    explicit Text(const QString& text = "", QWidget* parent = nullptr);
    virtual ~Text() = default;
    
    // Static factory
    static Text* create(const QString& text, Widget* parent = nullptr);
    
    // Content
    void setText(const QString& text);
    void setHtml(const QString& html);
    void appendText(const QString& text);
    
    // Font
    void setFont(const QString& family, int size = 12);
    void setFontSize(int size);
    void setBold(bool bold = true);
    void setItalic(bool italic = true);
    void setUnderline(bool underline = true);
    void setStrikethrough(bool strike = true);
    
    // Color
    void setColor(const QString& color);
    void setColor(const QColor& color);
    void setColor(int r, int g, int b, int a = 255);
    
    // Effects
    void setGlow(const QString& color, int radius = 5);
    void setGlow(const QColor& color, int radius = 5);
    void setShadow(const QColor& color, int blur = 3, int offsetX = 1, int offsetY = 1);
    
    // Alignment
    void setAlign(const QString& alignment);  // "left", "center", "right"
    void setAlign(Alignment align);
    void setVerticalAlign(Alignment align);
    
    // Preset styles
    void setTitle();
    void setSubtitle();
    void setBody();
    void setCaption();
    void setMonospace();
    void setCode();
    
    // Style class
    void setStyleClass(const QString& className);
    
    // Word wrap
    void setWrap(bool enabled);
    void setMaxWidth(int width);
    void setMaxLines(int lines);
    void setEllipsis(bool enabled);
    
private:
    QString m_styleClass;
    int m_maxLines = 0;
    bool m_ellipsis = false;
};

// ============================================================================
// PROGRESS BAR WIDGET
// ============================================================================
class ProgressBar : public QWidget {
    Q_OBJECT
    Q_PROPERTY(double value READ value WRITE setValue NOTIFY valueChanged)
    
public:
    explicit ProgressBar(QWidget* parent = nullptr);
    virtual ~ProgressBar() = default;
    
    // Static factory
    static ProgressBar* create(Widget* parent = nullptr);
    
    // Value
    void setValue(double value);
    double value() const { return m_value; }
    
    void setMinValue(double min);
    void setMaxValue(double max);
    void setRange(double min, double max);
    double minValue() const { return m_minValue; }
    double maxValue() const { return m_maxValue; }
    
    // Appearance
    void setColors(const QString& background, const QString& fill);
    void setBackgroundColor(const QColor& color);
    void setFillColor(const QColor& color);
    void setGradient(const QColor& start, const QColor& end);
    
    void setRounded(int radius);
    void setHeight(int height);
    
    // Text
    void setShowText(bool show);
    void setTextFormat(const QString& format);  // e.g., "%v%" or "%v / %m"
    void setTextColor(const QColor& color);
    
    // Animation
    void setAnimated(bool animated);
    void animateTo(double value, int duration = 300);
    
    // Style
    void setIndeterminate(bool enabled);
    void setOrientation(Qt::Orientation orient);
    
signals:
    void valueChanged(double value);
    
protected:
    void paintEvent(QPaintEvent* event) override;
    void timerEvent(QTimerEvent* event) override;
    
private:
    double m_value = 0;
    double m_minValue = 0;
    double m_maxValue = 100;
    double m_displayValue = 0;  // For animation
    
    QColor m_bgColor = QColor(60, 60, 70, 150);
    QColor m_fillColor = QColor(0, 150, 255);
    QColor m_fillEndColor;  // For gradient
    QColor m_textColor = Qt::white;
    
    int m_radius = 4;
    bool m_showText = false;
    QString m_textFormat = "%v%";
    bool m_animated = true;
    bool m_indeterminate = false;
    Qt::Orientation m_orientation = Qt::Horizontal;
    
    int m_animTimerId = 0;
    double m_animProgress = 0;
};

// ============================================================================
// GRAPH WIDGET
// ============================================================================
class Graph : public QWidget {
    Q_OBJECT
    
public:
    explicit Graph(QWidget* parent = nullptr);
    virtual ~Graph() = default;
    
    static Graph* create(Widget* parent = nullptr);
    
    // Data
    void addValue(double value);
    void setValues(const QList<double>& values);
    void clear();
    
    // Range
    void setMinValue(double min);
    void setMaxValue(double max);
    void setAutoScale(bool enabled);
    
    // Points
    void setMaxPoints(int count);
    int maxPoints() const { return m_maxPoints; }
    
    // Appearance
    void setGraphType(GraphType type);
    void setLineColor(const QColor& color);
    void setFillColor(const QColor& color);
    void setLineWidth(int width);
    void setShowGrid(bool show);
    void setGridColor(const QColor& color);
    void setShowLabels(bool show);
    
    // Style
    void setSmooth(bool enabled);
    void setAntialiased(bool enabled);
    
protected:
    void paintEvent(QPaintEvent* event) override;
    
private:
    void drawLine(QPainter& p);
    void drawArea(QPainter& p);
    void drawBar(QPainter& p);
    void drawGrid(QPainter& p);
    
private:
    QList<double> m_values;
    int m_maxPoints = 60;
    double m_minValue = 0;
    double m_maxValue = 100;
    bool m_autoScale = false;
    
    GraphType m_type = GraphType::Line;
    QColor m_lineColor = QColor(0, 200, 255);
    QColor m_fillColor = QColor(0, 200, 255, 50);
    QColor m_gridColor = QColor(255, 255, 255, 30);
    int m_lineWidth = 2;
    bool m_showGrid = true;
    bool m_showLabels = false;
    bool m_smooth = true;
    bool m_antialiased = true;
};

// ============================================================================
// GAUGE WIDGET
// ============================================================================
class Gauge : public QWidget {
    Q_OBJECT
    
public:
    explicit Gauge(QWidget* parent = nullptr);
    virtual ~Gauge() = default;
    
    static Gauge* create(Widget* parent = nullptr);
    
    // Value
    void setValue(double value);
    double value() const { return m_value; }
    void setRange(double min, double max);
    
    // Appearance
    void setStyle(GaugeStyle style);
    void setColors(const QColor& background, const QColor& fill);
    void setGradient(const QColor& start, const QColor& end);
    void setThickness(int thickness);
    
    // Arc settings
    void setStartAngle(int degrees);
    void setEndAngle(int degrees);
    
    // Labels
    void setShowValue(bool show);
    void setValueFormat(const QString& format);
    void setLabel(const QString& label);
    void setUnit(const QString& unit);
    void setTextColor(const QColor& color);
    
    // Animation
    void setAnimated(bool enabled);
    void animateTo(double value, int duration = 300);
    
signals:
    void valueChanged(double value);
    
protected:
    void paintEvent(QPaintEvent* event) override;
    
private:
    double m_value = 0;
    double m_minValue = 0;
    double m_maxValue = 100;
    double m_displayValue = 0;
    
    GaugeStyle m_style = GaugeStyle::Arc;
    QColor m_bgColor = QColor(60, 60, 70, 150);
    QColor m_fillColor = QColor(0, 200, 255);
    QColor m_fillEndColor;
    QColor m_textColor = Qt::white;
    int m_thickness = 10;
    int m_startAngle = 225;
    int m_endAngle = -45;
    
    bool m_showValue = true;
    QString m_valueFormat = "%.0f";
    QString m_label;
    QString m_unit;
    bool m_animated = true;
};

// ============================================================================
// IMAGE WIDGET
// ============================================================================
class Image : public QWidget {
    Q_OBJECT
    
public:
    explicit Image(QWidget* parent = nullptr);
    virtual ~Image() = default;
    
    static Image* create(Widget* parent = nullptr);
    static Image* create(const QString& path, Widget* parent = nullptr);
    
    // Source
    void setSource(const QString& path);
    void setSource(const QImage& image);
    void setSource(const QPixmap& pixmap);
    void setUrl(const QString& url);  // Load from network
    
    // Sizing
    void setFillMode(Qt::AspectRatioMode mode);
    void setRounded(int radius);
    void setCircular(bool circular);
    
    // Effects
    void setOpacity(double opacity);
    void setGrayscale(bool enabled);
    void setBlur(double radius);
    void setTint(const QColor& color);
    
    // Animation
    void setGif(const QString& path);
    
signals:
    void loaded();
    void loadError(const QString& error);
    
protected:
    void paintEvent(QPaintEvent* event) override;
    
private:
    QPixmap m_pixmap;
    QString m_source;
    Qt::AspectRatioMode m_fillMode = Qt::KeepAspectRatio;
    int m_radius = 0;
    bool m_circular = false;
    double m_opacity = 1.0;
    bool m_grayscale = false;
    double m_blurRadius = 0;
    QColor m_tint;
};

// ============================================================================
// BUTTON WIDGET
// ============================================================================
class Button : public QPushButton {
    Q_OBJECT
    
public:
    explicit Button(const QString& text = "", QWidget* parent = nullptr);
    virtual ~Button() = default;
    
    static Button* create(const QString& text, Widget* parent = nullptr);
    
    // Appearance
    void setBackground(const QColor& color);
    void setHoverBackground(const QColor& color);
    void setPressedBackground(const QColor& color);
    void setTextColor(const QColor& color);
    void setRounded(int radius);
    void setBorder(const QColor& color, int width = 1);
    
    // Icon
    void setIcon(const QString& path);
    void setIconSize(int size);
    
    // Callback
    void onClick(ClickCallback callback);
    
protected:
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    
private:
    void updateStyle();
    
private:
    QColor m_bgColor = QColor(60, 60, 80, 200);
    QColor m_hoverColor = QColor(80, 80, 100, 200);
    QColor m_pressedColor = QColor(50, 50, 70, 200);
    QColor m_textColor = Qt::white;
    QColor m_borderColor = Qt::transparent;
    int m_borderWidth = 0;
    int m_radius = 6;
    bool m_hovered = false;
    
    ClickCallback m_onClick;
};

// ============================================================================
// SPACER WIDGET
// ============================================================================
class Spacer : public QWidget {
    Q_OBJECT
    
public:
    explicit Spacer(int size = 0, QWidget* parent = nullptr);
    virtual ~Spacer() = default;
    
    static Spacer* create(int size, Widget* parent = nullptr);
    static Spacer* horizontal(int size, Widget* parent = nullptr);
    static Spacer* vertical(int size, Widget* parent = nullptr);
    
    void setSize(int size);
    void setExpanding(bool horizontal, bool vertical);
};

// ============================================================================
// CONTAINER WIDGET
// ============================================================================
class Container : public QWidget {
    Q_OBJECT
    
public:
    enum Layout { Vertical, Horizontal, Grid };
    
    explicit Container(Layout layout = Vertical, QWidget* parent = nullptr);
    virtual ~Container() = default;
    
    static Container* create(Layout layout, Widget* parent = nullptr);
    static Container* vbox(Widget* parent = nullptr);
    static Container* hbox(Widget* parent = nullptr);
    static Container* grid(Widget* parent = nullptr);
    
    void addWidget(QWidget* widget);
    void addWidget(QWidget* widget, int row, int col);
    void addSpacing(int size);
    void addStretch(int factor = 1);
    
    void setSpacing(int spacing);
    void setMargins(int margin);
    void setMargins(int top, int right, int bottom, int left);
    
protected:
    void paintEvent(QPaintEvent* event) override;
    
private:
    Layout m_layout;
    QLayout* m_layoutPtr = nullptr;
    QColor m_bgColor = Qt::transparent;
};

// ============================================================================
// CLOCK WIDGET
// ============================================================================
class Clock : public QWidget {
    Q_OBJECT
    
public:
    enum Style { Digital, Analog, Minimal };
    
    explicit Clock(Style style = Digital, QWidget* parent = nullptr);
    virtual ~Clock() = default;
    
    static Clock* create(Style style, Widget* parent = nullptr);
    
    // Format
    void setFormat(const QString& format);  // Qt date/time format
    void set24Hour(bool enabled);
    void setShowSeconds(bool show);
    void setShowDate(bool show);
    void setDateFormat(const QString& format);
    
    // Appearance
    void setTextColor(const QColor& color);
    void setFont(const QString& family, int size = 24);
    
    // Analog
    void setHandColors(const QColor& hour, const QColor& minute, const QColor& second);
    void setDialColor(const QColor& color);
    void setShowTicks(bool show);
    
    // Timezone
    void setTimezone(const QString& tz);
    
protected:
    void paintEvent(QPaintEvent* event) override;
    void timerEvent(QTimerEvent* event) override;
    
private:
    void drawDigital(QPainter& p);
    void drawAnalog(QPainter& p);
    void drawMinimal(QPainter& p);
    
private:
    Style m_style;
    QString m_timeFormat = "hh:mm:ss";
    QString m_dateFormat = "dddd, MMMM d";
    bool m_24hour = true;
    bool m_showSeconds = true;
    bool m_showDate = true;
    
    QColor m_textColor = Qt::white;
    QColor m_hourHandColor = Qt::white;
    QColor m_minuteHandColor = QColor(200, 200, 200);
    QColor m_secondHandColor = QColor(255, 100, 100);
    QColor m_dialColor = QColor(40, 40, 50);
    bool m_showTicks = true;
    
    QString m_timezone;
    int m_timerId = 0;
};

// ============================================================================
// CALENDAR WIDGET
// ============================================================================
class Calendar : public QWidget {
    Q_OBJECT
    
public:
    explicit Calendar(QWidget* parent = nullptr);
    virtual ~Calendar() = default;
    
    static Calendar* create(Widget* parent = nullptr);
    
    // Date
    void setDate(const QDate& date);
    void setDate(int year, int month, int day);
    QDate date() const { return m_date; }
    
    // Navigation
    void nextMonth();
    void prevMonth();
    void goToToday();
    
    // Appearance
    void setHeaderColor(const QColor& color);
    void setDayColor(const QColor& color);
    void setTodayColor(const QColor& color);
    void setSelectedColor(const QColor& color);
    void setWeekendColor(const QColor& color);
    
    // Options
    void setShowWeekNumbers(bool show);
    void setFirstDayOfWeek(Qt::DayOfWeek day);
    void setHighlightToday(bool highlight);
    
signals:
    void dateSelected(const QDate& date);
    void monthChanged(int year, int month);
    
protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    
private:
    QDate m_date;
    QDate m_selectedDate;
    QColor m_headerColor = QColor(60, 130, 200);
    QColor m_dayColor = Qt::white;
    QColor m_todayColor = QColor(255, 180, 100);
    QColor m_selectedColor = QColor(100, 180, 255);
    QColor m_weekendColor = QColor(200, 100, 100);
    bool m_showWeekNumbers = false;
    Qt::DayOfWeek m_firstDay = Qt::Monday;
    bool m_highlightToday = true;
};

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

// Quick creation
Text* text(const QString& content, Widget* parent = nullptr);
Text* title(const QString& content, Widget* parent = nullptr);
Text* label(const QString& content, Widget* parent = nullptr);

ProgressBar* progressBar(Widget* parent = nullptr);
Graph* graph(Widget* parent = nullptr);
Gauge* gauge(Widget* parent = nullptr);
Image* image(const QString& path, Widget* parent = nullptr);
Button* button(const QString& text, Widget* parent = nullptr);
Spacer* spacer(int size = 0, Widget* parent = nullptr);
Container* container(Container::Layout layout = Container::Vertical, Widget* parent = nullptr);
Clock* clock(Clock::Style style = Clock::Digital, Widget* parent = nullptr);
Calendar* calendar(Widget* parent = nullptr);

} // namespace Milk
