/**
 * MilkWidgetCore - Widget Implementations
 */

#include "milk/Widgets.h"
#include "milk/Widget.h"
#include "milk/Utils.h"

#include <QPainter>
#include <QPainterPath>
#include <QGraphicsDropShadowEffect>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFontDatabase>
#include <QtMath>

namespace Milk {

// ============================================================================
// TEXT WIDGET
// ============================================================================

Text::Text(const QString& text, QWidget* parent) : QLabel(text, parent) {
    setWordWrap(true);
    setAttribute(Qt::WA_TranslucentBackground);
    setStyleSheet("background: transparent;");
}

Text* Text::create(const QString& text, Widget* parent) { return new Text(text, parent); }
void Text::setText(const QString& text) { QLabel::setText(text); }
void Text::setHtml(const QString& html) { QLabel::setText(html); setTextFormat(Qt::RichText); }
void Text::appendText(const QString& text) { QLabel::setText(QLabel::text() + text); }

void Text::setFont(const QString& family, int size) {
    QFont f = font(); f.setFamily(family); f.setPointSize(size); QLabel::setFont(f);
}
void Text::setFontSize(int size) { QFont f = font(); f.setPointSize(size); QLabel::setFont(f); }
void Text::setBold(bool bold) { QFont f = font(); f.setBold(bold); QLabel::setFont(f); }
void Text::setItalic(bool italic) { QFont f = font(); f.setItalic(italic); QLabel::setFont(f); }
void Text::setUnderline(bool underline) { QFont f = font(); f.setUnderline(underline); QLabel::setFont(f); }
void Text::setStrikethrough(bool strike) { QFont f = font(); f.setStrikeOut(strike); QLabel::setFont(f); }

void Text::setColor(const QString& color) { setColor(Color::parse(color)); }
void Text::setColor(const QColor& color) { QPalette p = palette(); p.setColor(QPalette::WindowText, color); setPalette(p); }
void Text::setColor(int r, int g, int b, int a) { setColor(QColor(r, g, b, a)); }

void Text::setGlow(const QString& color, int radius) { setGlow(Color::parse(color), radius); }
void Text::setGlow(const QColor& color, int radius) {
    auto* effect = new QGraphicsDropShadowEffect(this);
    effect->setBlurRadius(radius * 2); effect->setColor(color); effect->setOffset(0, 0);
    setGraphicsEffect(effect);
}
void Text::setShadow(const QColor& color, int blur, int offsetX, int offsetY) {
    auto* effect = new QGraphicsDropShadowEffect(this);
    effect->setBlurRadius(blur); effect->setColor(color); effect->setOffset(offsetX, offsetY);
    setGraphicsEffect(effect);
}

void Text::setAlign(const QString& alignment) {
    QString a = alignment.toLower();
    if (a == "left") setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    else if (a == "center") setAlignment(Qt::AlignCenter);
    else if (a == "right") setAlignment(Qt::AlignRight | Qt::AlignVCenter);
}
void Text::setAlign(Alignment align) {
    switch (align) {
        case Alignment::Left: setAlignment(Qt::AlignLeft | Qt::AlignVCenter); break;
        case Alignment::Center: setAlignment(Qt::AlignCenter); break;
        case Alignment::Right: setAlignment(Qt::AlignRight | Qt::AlignVCenter); break;
        default: break;
    }
}
void Text::setVerticalAlign(Alignment align) {
    Qt::Alignment current = alignment() & Qt::AlignHorizontal_Mask;
    switch (align) {
        case Alignment::Top: setAlignment(current | Qt::AlignTop); break;
        case Alignment::Center: setAlignment(current | Qt::AlignVCenter); break;
        case Alignment::Bottom: setAlignment(current | Qt::AlignBottom); break;
        default: break;
    }
}

void Text::setTitle() { QFont f = font(); f.setPointSize(18); f.setBold(true); QLabel::setFont(f); }
void Text::setSubtitle() { QFont f = font(); f.setPointSize(14); QLabel::setFont(f); }
void Text::setBody() { QFont f = font(); f.setPointSize(12); QLabel::setFont(f); }
void Text::setCaption() { QFont f = font(); f.setPointSize(10); QLabel::setFont(f); setColor(QColor(150,150,150)); }
void Text::setMonospace() { QFont f = QFontDatabase::systemFont(QFontDatabase::FixedFont); f.setPointSize(font().pointSize()); QLabel::setFont(f); }
void Text::setCode() { setMonospace(); setStyleSheet("background: rgba(0,0,0,50); padding: 4px; border-radius: 3px;"); }
void Text::setStyleClass(const QString& className) { m_styleClass = className; }
void Text::setWrap(bool enabled) { setWordWrap(enabled); }
void Text::setMaxWidth(int width) { setMaximumWidth(width); }
void Text::setMaxLines(int lines) { m_maxLines = lines; }
void Text::setEllipsis(bool enabled) { m_ellipsis = enabled; }

// ============================================================================
// PROGRESS BAR
// ============================================================================

ProgressBar::ProgressBar(QWidget* parent) : QWidget(parent) {
    setMinimumHeight(8); setMaximumHeight(30);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}
ProgressBar* ProgressBar::create(Widget* parent) { return new ProgressBar(parent); }

void ProgressBar::setValue(double value) {
    m_value = qBound(m_minValue, value, m_maxValue);
    if (m_animated && m_animTimerId == 0) m_animTimerId = startTimer(16);
    else if (!m_animated) m_displayValue = m_value;
    emit valueChanged(m_value); update();
}
void ProgressBar::setMinValue(double min) { m_minValue = min; update(); }
void ProgressBar::setMaxValue(double max) { m_maxValue = max; update(); }
void ProgressBar::setRange(double min, double max) { m_minValue = min; m_maxValue = max; update(); }
void ProgressBar::setColors(const QString& bg, const QString& fill) { m_bgColor = Color::parse(bg); m_fillColor = Color::parse(fill); update(); }
void ProgressBar::setBackgroundColor(const QColor& c) { m_bgColor = c; update(); }
void ProgressBar::setFillColor(const QColor& c) { m_fillColor = c; update(); }
void ProgressBar::setGradient(const QColor& start, const QColor& end) { m_fillColor = start; m_fillEndColor = end; update(); }
void ProgressBar::setRounded(int r) { m_radius = r; update(); }
void ProgressBar::setHeight(int h) { setFixedHeight(h); }
void ProgressBar::setShowText(bool show) { m_showText = show; update(); }
void ProgressBar::setTextFormat(const QString& fmt) { m_textFormat = fmt; update(); }
void ProgressBar::setTextColor(const QColor& c) { m_textColor = c; update(); }
void ProgressBar::setAnimated(bool a) { m_animated = a; }
void ProgressBar::animateTo(double v, int) { setValue(v); }
void ProgressBar::setIndeterminate(bool e) { m_indeterminate = e; if (e && m_animTimerId == 0) m_animTimerId = startTimer(16); update(); }
void ProgressBar::setOrientation(Qt::Orientation o) { m_orientation = o; update(); }

void ProgressBar::paintEvent(QPaintEvent*) {
    QPainter p(this); p.setRenderHint(QPainter::Antialiasing);
    QRectF r = rect();
    QPainterPath bgPath; bgPath.addRoundedRect(r, m_radius, m_radius); p.fillPath(bgPath, m_bgColor);
    double range = m_maxValue - m_minValue;
    double pct = range > 0 ? (m_displayValue - m_minValue) / range : 0;
    pct = qBound(0.0, pct, 1.0);
    QRectF fillRect = r; fillRect.setWidth(r.width() * pct);
    QPainterPath fillPath; fillPath.addRoundedRect(fillRect, m_radius, m_radius);
    if (m_fillEndColor.isValid()) {
        QLinearGradient grad(fillRect.topLeft(), fillRect.topRight());
        grad.setColorAt(0, m_fillColor); grad.setColorAt(1, m_fillEndColor);
        p.fillPath(fillPath, grad);
    } else p.fillPath(fillPath, m_fillColor);
    if (m_showText) {
        QString txt = m_textFormat;
        txt.replace("%v", QString::number(int(m_value)));
        txt.replace("%m", QString::number(int(m_maxValue)));
        p.setPen(m_textColor); p.drawText(r, Qt::AlignCenter, txt);
    }
}
void ProgressBar::timerEvent(QTimerEvent* e) {
    if (e->timerId() == m_animTimerId) {
        double diff = m_value - m_displayValue;
        if (qAbs(diff) < 0.1) { m_displayValue = m_value; killTimer(m_animTimerId); m_animTimerId = 0; }
        else m_displayValue += diff * 0.15;
        update();
    }
}

// ============================================================================
// GRAPH
// ============================================================================

Graph::Graph(QWidget* parent) : QWidget(parent) {
    setMinimumHeight(50); setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}
Graph* Graph::create(Widget* parent) { return new Graph(parent); }

void Graph::addValue(double value) {
    m_values.append(value);
    while (m_values.size() > m_maxPoints) m_values.removeFirst();
    if (m_autoScale && !m_values.isEmpty()) {
        m_minValue = *std::min_element(m_values.begin(), m_values.end());
        m_maxValue = *std::max_element(m_values.begin(), m_values.end());
        if (qFuzzyCompare(m_minValue, m_maxValue)) m_maxValue = m_minValue + 1;
    }
    update();
}
void Graph::setValues(const QList<double>& v) { m_values = v; while (m_values.size() > m_maxPoints) m_values.removeFirst(); update(); }
void Graph::clear() { m_values.clear(); update(); }
void Graph::setMinValue(double m) { m_minValue = m; update(); }
void Graph::setMaxValue(double m) { m_maxValue = m; update(); }
void Graph::setAutoScale(bool e) { m_autoScale = e; }
void Graph::setMaxPoints(int c) { m_maxPoints = c; }
void Graph::setGraphType(GraphType t) { m_type = t; update(); }
void Graph::setLineColor(const QColor& c) { m_lineColor = c; update(); }
void Graph::setFillColor(const QColor& c) { m_fillColor = c; update(); }
void Graph::setLineWidth(int w) { m_lineWidth = w; update(); }
void Graph::setShowGrid(bool s) { m_showGrid = s; update(); }
void Graph::setGridColor(const QColor& c) { m_gridColor = c; update(); }
void Graph::setShowLabels(bool s) { m_showLabels = s; update(); }
void Graph::setSmooth(bool e) { m_smooth = e; update(); }
void Graph::setAntialiased(bool e) { m_antialiased = e; update(); }

void Graph::paintEvent(QPaintEvent*) {
    QPainter p(this); p.setRenderHint(QPainter::Antialiasing, m_antialiased);
    if (m_showGrid) drawGrid(p);
    switch (m_type) {
        case GraphType::Line: case GraphType::Sparkline: drawLine(p); break;
        case GraphType::Area: drawArea(p); break;
        case GraphType::Bar: drawBar(p); break;
    }
}
void Graph::drawGrid(QPainter& p) {
    p.setPen(QPen(m_gridColor, 1));
    for (int i = 1; i < 4; i++) { int y = height() * i / 4; p.drawLine(0, y, width(), y); }
    for (int i = 1; i < 4; i++) { int x = width() * i / 4; p.drawLine(x, 0, x, height()); }
}
void Graph::drawLine(QPainter& p) {
    if (m_values.size() < 2) return;
    double range = m_maxValue - m_minValue; if (range <= 0) range = 1;
    double xStep = double(width()) / (m_maxPoints - 1);
    QPainterPath path;
    for (int i = 0; i < m_values.size(); i++) {
        double x = i * xStep, y = height() - (m_values[i] - m_minValue) / range * height();
        if (i == 0) path.moveTo(x, y); else path.lineTo(x, y);
    }
    p.setPen(QPen(m_lineColor, m_lineWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p.drawPath(path);
}
void Graph::drawArea(QPainter& p) {
    if (m_values.size() < 2) return;
    double range = m_maxValue - m_minValue; if (range <= 0) range = 1;
    double xStep = double(width()) / (m_maxPoints - 1);
    QPainterPath path; path.moveTo(0, height());
    for (int i = 0; i < m_values.size(); i++) {
        double x = i * xStep, y = height() - (m_values[i] - m_minValue) / range * height();
        path.lineTo(x, y);
    }
    path.lineTo((m_values.size() - 1) * xStep, height()); path.closeSubpath();
    QLinearGradient grad(0, 0, 0, height());
    grad.setColorAt(0, m_fillColor);
    grad.setColorAt(1, QColor(m_fillColor.red(), m_fillColor.green(), m_fillColor.blue(), 20));
    p.fillPath(path, grad); drawLine(p);
}
void Graph::drawBar(QPainter& p) {
    if (m_values.isEmpty()) return;
    double range = m_maxValue - m_minValue; if (range <= 0) range = 1;
    double barW = double(width()) / m_values.size() - 2;
    for (int i = 0; i < m_values.size(); i++) {
        double x = i * (barW + 2), barH = (m_values[i] - m_minValue) / range * height();
        p.fillRect(QRectF(x, height() - barH, barW, barH), m_lineColor);
    }
}

// ============================================================================
// GAUGE
// ============================================================================

Gauge::Gauge(QWidget* parent) : QWidget(parent) {
    setMinimumSize(80, 80); setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}
Gauge* Gauge::create(Widget* parent) { return new Gauge(parent); }

void Gauge::setValue(double v) {
    m_value = qBound(m_minValue, v, m_maxValue);
    m_displayValue = m_animated ? m_displayValue + (m_value - m_displayValue) * 0.3 : m_value;
    emit valueChanged(m_value); update();
}
void Gauge::setRange(double min, double max) { m_minValue = min; m_maxValue = max; update(); }
void Gauge::setStyle(GaugeStyle s) { m_style = s; update(); }
void Gauge::setColors(const QColor& bg, const QColor& fill) { m_bgColor = bg; m_fillColor = fill; update(); }
void Gauge::setGradient(const QColor& s, const QColor& e) { m_fillColor = s; m_fillEndColor = e; update(); }
void Gauge::setThickness(int t) { m_thickness = t; update(); }
void Gauge::setStartAngle(int d) { m_startAngle = d; update(); }
void Gauge::setEndAngle(int d) { m_endAngle = d; update(); }
void Gauge::setShowValue(bool s) { m_showValue = s; update(); }
void Gauge::setValueFormat(const QString& f) { m_valueFormat = f; update(); }
void Gauge::setLabel(const QString& l) { m_label = l; update(); }
void Gauge::setUnit(const QString& u) { m_unit = u; update(); }
void Gauge::setTextColor(const QColor& c) { m_textColor = c; update(); }
void Gauge::setAnimated(bool e) { m_animated = e; }
void Gauge::animateTo(double v, int) { setValue(v); }

void Gauge::paintEvent(QPaintEvent*) {
    QPainter p(this); p.setRenderHint(QPainter::Antialiasing);
    int side = qMin(width(), height());
    QRectF r((width()-side)/2.0, (height()-side)/2.0, side, side);
    r.adjust(m_thickness, m_thickness, -m_thickness, -m_thickness);
    double range = m_maxValue - m_minValue; if (range <= 0) range = 1;
    double pct = (m_displayValue - m_minValue) / range;
    int spanAngle = m_startAngle - m_endAngle, valueAngle = int(spanAngle * pct);
    p.setPen(QPen(m_bgColor, m_thickness, Qt::SolidLine, Qt::RoundCap));
    p.drawArc(r, m_startAngle * 16, -spanAngle * 16);
    p.setPen(QPen(m_fillColor, m_thickness, Qt::SolidLine, Qt::RoundCap));
    p.drawArc(r, m_startAngle * 16, -valueAngle * 16);
    if (m_showValue) {
        p.setPen(m_textColor);
        QFont f = p.font(); f.setPointSize(side/6); f.setBold(true); p.setFont(f);
        QString txt = QString::asprintf(m_valueFormat.toUtf8().constData(), m_displayValue);
        if (!m_unit.isEmpty()) txt += m_unit;
        p.drawText(r, Qt::AlignCenter, txt);
    }
}

// ============================================================================
// IMAGE
// ============================================================================

Image::Image(QWidget* parent) : QWidget(parent) { setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding); }
Image* Image::create(Widget* parent) { return new Image(parent); }
Image* Image::create(const QString& path, Widget* parent) { Image* img = new Image(parent); img->setSource(path); return img; }
void Image::setSource(const QString& path) { m_source = path; m_pixmap = QPixmap(path); update(); }
void Image::setSource(const QImage& image) { m_pixmap = QPixmap::fromImage(image); update(); }
void Image::setSource(const QPixmap& pixmap) { m_pixmap = pixmap; update(); }
void Image::setUrl(const QString&) { }
void Image::setFillMode(Qt::AspectRatioMode m) { m_fillMode = m; update(); }
void Image::setRounded(int r) { m_radius = r; update(); }
void Image::setCircular(bool c) { m_circular = c; update(); }
void Image::setOpacity(double o) { m_opacity = o; update(); }
void Image::setGrayscale(bool) { update(); }
void Image::setBlur(double r) { m_blurRadius = r; update(); }
void Image::setTint(const QColor& c) { m_tint = c; update(); }
void Image::setGif(const QString&) { }

void Image::paintEvent(QPaintEvent*) {
    if (m_pixmap.isNull()) return;
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    p.setOpacity(m_opacity);
    QPixmap scaled = m_pixmap.scaled(size(), m_fillMode, Qt::SmoothTransformation);
    QRectF r((width()-scaled.width())/2.0, (height()-scaled.height())/2.0, scaled.width(), scaled.height());
    if (m_circular || m_radius > 0) {
        QPainterPath path;
        if (m_circular) path.addEllipse(r); else path.addRoundedRect(r, m_radius, m_radius);
        p.setClipPath(path);
    }
    p.drawPixmap(r.toRect(), scaled);
}

// ============================================================================
// BUTTON
// ============================================================================

Button::Button(const QString& text, QWidget* parent) : QPushButton(text, parent) { setCursor(Qt::PointingHandCursor); updateStyle(); }
Button* Button::create(const QString& text, Widget* parent) { return new Button(text, parent); }
void Button::setBackground(const QColor& c) { m_bgColor = c; updateStyle(); }
void Button::setHoverBackground(const QColor& c) { m_hoverColor = c; updateStyle(); }
void Button::setPressedBackground(const QColor& c) { m_pressedColor = c; updateStyle(); }
void Button::setTextColor(const QColor& c) { m_textColor = c; updateStyle(); }
void Button::setRounded(int r) { m_radius = r; updateStyle(); }
void Button::setBorder(const QColor& c, int w) { m_borderColor = c; m_borderWidth = w; updateStyle(); }
void Button::setIcon(const QString& p) { QPushButton::setIcon(QIcon(p)); }
void Button::setIconSize(int s) { QPushButton::setIconSize(QSize(s, s)); }
void Button::onClick(ClickCallback cb) { m_onClick = cb; connect(this, &QPushButton::clicked, cb); }
void Button::enterEvent(QEnterEvent*) { m_hovered = true; updateStyle(); }
void Button::leaveEvent(QEvent*) { m_hovered = false; updateStyle(); }
void Button::paintEvent(QPaintEvent* e) { QPushButton::paintEvent(e); }
void Button::updateStyle() {
    QColor bg = m_hovered ? m_hoverColor : m_bgColor;
    QString ss = QString("QPushButton{background:%1;color:%2;border-radius:%3px;padding:8px 16px;border:%4px solid %5}"
                        "QPushButton:pressed{background:%6}")
        .arg(Color::toRgba(bg)).arg(Color::toRgba(m_textColor)).arg(m_radius)
        .arg(m_borderWidth).arg(Color::toRgba(m_borderColor)).arg(Color::toRgba(m_pressedColor));
    setStyleSheet(ss);
}

// ============================================================================
// SPACER
// ============================================================================

Spacer::Spacer(int size, QWidget* parent) : QWidget(parent) {
    if (size > 0) setFixedSize(size, size);
    else setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}
Spacer* Spacer::create(int size, Widget* parent) { return new Spacer(size, parent); }
Spacer* Spacer::horizontal(int size, Widget* parent) { Spacer* s = new Spacer(0, parent); s->setFixedWidth(size); s->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum); return s; }
Spacer* Spacer::vertical(int size, Widget* parent) { Spacer* s = new Spacer(0, parent); s->setFixedHeight(size); s->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed); return s; }
void Spacer::setSize(int s) { setFixedSize(s, s); }
void Spacer::setExpanding(bool h, bool v) { setSizePolicy(h ? QSizePolicy::Expanding : QSizePolicy::Minimum, v ? QSizePolicy::Expanding : QSizePolicy::Minimum); }

// ============================================================================
// CONTAINER
// ============================================================================

Container::Container(Layout layout, QWidget* parent) : QWidget(parent), m_layout(layout) {
    setAttribute(Qt::WA_TranslucentBackground);
    switch (layout) {
        case Vertical: m_layoutPtr = new QVBoxLayout(this); break;
        case Horizontal: m_layoutPtr = new QHBoxLayout(this); break;
        case Grid: m_layoutPtr = new QGridLayout(this); break;
    }
    m_layoutPtr->setContentsMargins(0, 0, 0, 0); m_layoutPtr->setSpacing(5);
}
Container* Container::create(Layout l, Widget* p) { return new Container(l, p); }
Container* Container::vbox(Widget* p) { return new Container(Vertical, p); }
Container* Container::hbox(Widget* p) { return new Container(Horizontal, p); }
Container* Container::grid(Widget* p) { return new Container(Grid, p); }
void Container::addWidget(QWidget* w) {
    if (m_layout == Grid) static_cast<QGridLayout*>(m_layoutPtr)->addWidget(w);
    else static_cast<QBoxLayout*>(m_layoutPtr)->addWidget(w);
}
void Container::addWidget(QWidget* w, int row, int col) {
    if (m_layout == Grid) static_cast<QGridLayout*>(m_layoutPtr)->addWidget(w, row, col);
    else addWidget(w);
}
void Container::addSpacing(int s) { if (m_layout != Grid) static_cast<QBoxLayout*>(m_layoutPtr)->addSpacing(s); }
void Container::addStretch(int f) { if (m_layout != Grid) static_cast<QBoxLayout*>(m_layoutPtr)->addStretch(f); }
void Container::setSpacing(int s) { m_layoutPtr->setSpacing(s); }
void Container::setMargins(int m) { m_layoutPtr->setContentsMargins(m, m, m, m); }
void Container::setMargins(int t, int r, int b, int l) { m_layoutPtr->setContentsMargins(l, t, r, b); }
void Container::paintEvent(QPaintEvent*) { if (m_bgColor.alpha() > 0) { QPainter p(this); p.fillRect(rect(), m_bgColor); } }

// ============================================================================
// CLOCK
// ============================================================================

Clock::Clock(Style style, QWidget* parent) : QWidget(parent), m_style(style) {
    m_timerId = startTimer(1000); setMinimumSize(100, style == Analog ? 100 : 40);
}
Clock* Clock::create(Style s, Widget* p) { return new Clock(s, p); }
void Clock::setFormat(const QString& f) { m_timeFormat = f; update(); }
void Clock::set24Hour(bool e) { m_24hour = e; m_timeFormat = e ? "hh:mm:ss" : "h:mm:ss AP"; update(); }
void Clock::setShowSeconds(bool s) { m_showSeconds = s; m_timeFormat = m_24hour ? (s ? "hh:mm:ss" : "hh:mm") : (s ? "h:mm:ss AP" : "h:mm AP"); update(); }
void Clock::setShowDate(bool s) { m_showDate = s; update(); }
void Clock::setDateFormat(const QString& f) { m_dateFormat = f; update(); }
void Clock::setTextColor(const QColor& c) { m_textColor = c; update(); }
void Clock::setFont(const QString& family, int size) { QFont f(family, size); QWidget::setFont(f); update(); }
void Clock::setHandColors(const QColor& h, const QColor& m, const QColor& s) { m_hourHandColor = h; m_minuteHandColor = m; m_secondHandColor = s; update(); }
void Clock::setDialColor(const QColor& c) { m_dialColor = c; update(); }
void Clock::setShowTicks(bool s) { m_showTicks = s; update(); }
void Clock::setTimezone(const QString& tz) { m_timezone = tz; update(); }
void Clock::timerEvent(QTimerEvent* e) { if (e->timerId() == m_timerId) update(); }

void Clock::paintEvent(QPaintEvent*) {
    QPainter p(this); p.setRenderHint(QPainter::Antialiasing);
    switch (m_style) { case Digital: drawDigital(p); break; case Analog: drawAnalog(p); break; case Minimal: drawMinimal(p); break; }
}
void Clock::drawDigital(QPainter& p) {
    QDateTime now = QDateTime::currentDateTime();
    p.setPen(m_textColor);
    QFont f = font(); f.setPointSize(height()/3); f.setBold(true); p.setFont(f);
    p.drawText(rect(), Qt::AlignCenter, now.toString(m_timeFormat));
    if (m_showDate) {
        f.setPointSize(height()/6); f.setBold(false); p.setFont(f);
        QRectF dr = rect(); dr.setTop(rect().center().y() + height()/6);
        p.drawText(dr, Qt::AlignHCenter | Qt::AlignTop, now.toString(m_dateFormat));
    }
}
void Clock::drawAnalog(QPainter& p) {
    int side = qMin(width(), height());
    p.translate(width()/2, height()/2); p.scale(side/200.0, side/200.0);
    p.setPen(Qt::NoPen); p.setBrush(m_dialColor); p.drawEllipse(QPoint(0,0), 95, 95);
    if (m_showTicks) { p.setPen(QPen(m_textColor, 2)); for (int i = 0; i < 12; i++) { p.drawLine(0, -88, 0, -78); p.rotate(30); } }
    QTime time = QTime::currentTime();
    p.save(); p.rotate(30.0 * (time.hour() + time.minute()/60.0));
    p.setPen(Qt::NoPen); p.setBrush(m_hourHandColor); p.drawConvexPolygon(QPolygon({{-4,0},{0,-50},{4,0}})); p.restore();
    p.save(); p.rotate(6.0 * (time.minute() + time.second()/60.0));
    p.setBrush(m_minuteHandColor); p.drawConvexPolygon(QPolygon({{-3,0},{0,-70},{3,0}})); p.restore();
    if (m_showSeconds) { p.save(); p.rotate(6.0 * time.second()); p.setPen(QPen(m_secondHandColor, 1)); p.drawLine(0, 10, 0, -80); p.restore(); }
    p.setBrush(m_textColor); p.drawEllipse(QPoint(0,0), 5, 5);
}
void Clock::drawMinimal(QPainter& p) {
    QDateTime now = QDateTime::currentDateTime();
    p.setPen(m_textColor);
    QFont f = font(); f.setPointSize(height()/2); p.setFont(f);
    p.drawText(rect(), Qt::AlignCenter, now.toString(m_showSeconds ? "hh:mm:ss" : "hh:mm"));
}

// ============================================================================
// CALENDAR
// ============================================================================

Calendar::Calendar(QWidget* parent) : QWidget(parent), m_date(QDate::currentDate()), m_selectedDate(QDate::currentDate()) { setMinimumSize(200, 180); }
Calendar* Calendar::create(Widget* p) { return new Calendar(p); }
void Calendar::setDate(const QDate& d) { m_date = d; update(); }
void Calendar::setDate(int y, int m, int d) { m_date = QDate(y, m, d); update(); }
void Calendar::nextMonth() { m_date = m_date.addMonths(1); emit monthChanged(m_date.year(), m_date.month()); update(); }
void Calendar::prevMonth() { m_date = m_date.addMonths(-1); emit monthChanged(m_date.year(), m_date.month()); update(); }
void Calendar::goToToday() { m_date = QDate::currentDate(); m_selectedDate = m_date; update(); }
void Calendar::setHeaderColor(const QColor& c) { m_headerColor = c; update(); }
void Calendar::setDayColor(const QColor& c) { m_dayColor = c; update(); }
void Calendar::setTodayColor(const QColor& c) { m_todayColor = c; update(); }
void Calendar::setSelectedColor(const QColor& c) { m_selectedColor = c; update(); }
void Calendar::setWeekendColor(const QColor& c) { m_weekendColor = c; update(); }
void Calendar::setShowWeekNumbers(bool s) { m_showWeekNumbers = s; update(); }
void Calendar::setFirstDayOfWeek(Qt::DayOfWeek d) { m_firstDay = d; update(); }
void Calendar::setHighlightToday(bool h) { m_highlightToday = h; update(); }

void Calendar::paintEvent(QPaintEvent*) {
    QPainter p(this); p.setRenderHint(QPainter::Antialiasing);
    int cellW = width()/7, cellH = (height()-30)/7;
    p.fillRect(0, 0, width(), 25, m_headerColor);
    p.setPen(Qt::white);
    QFont f = font(); f.setBold(true); p.setFont(f);
    p.drawText(QRect(0,0,width(),25), Qt::AlignCenter, m_date.toString("MMMM yyyy"));
    f.setBold(false); f.setPointSize(9); p.setFont(f);
    QStringList days = {"Mo","Tu","We","Th","Fr","Sa","Su"};
    for (int i = 0; i < 7; i++) { p.setPen(i >= 5 ? m_weekendColor : m_dayColor); p.drawText(QRect(i*cellW, 28, cellW, 20), Qt::AlignCenter, days[i]); }
    QDate first(m_date.year(), m_date.month(), 1);
    int startDay = first.dayOfWeek() - 1, daysInMonth = m_date.daysInMonth();
    QDate today = QDate::currentDate();
    f.setPointSize(10); p.setFont(f);
    for (int d = 1; d <= daysInMonth; d++) {
        int idx = startDay + d - 1, row = idx/7, col = idx%7;
        QRect r(col*cellW, 50+row*cellH, cellW, cellH);
        QDate thisDate(m_date.year(), m_date.month(), d);
        if (thisDate == m_selectedDate) { p.fillRect(r.adjusted(2,2,-2,-2), m_selectedColor); p.setPen(Qt::white); }
        else if (m_highlightToday && thisDate == today) p.setPen(m_todayColor);
        else p.setPen(col >= 5 ? m_weekendColor : m_dayColor);
        p.drawText(r, Qt::AlignCenter, QString::number(d));
    }
}
void Calendar::mousePressEvent(QMouseEvent* e) {
    int cellW = width()/7, cellH = (height()-30)/7;
    int col = e->pos().x()/cellW, row = (e->pos().y()-50)/cellH;
    if (row < 0) return;
    QDate first(m_date.year(), m_date.month(), 1);
    int day = row*7 + col - (first.dayOfWeek()-1) + 1;
    if (day >= 1 && day <= m_date.daysInMonth()) { m_selectedDate = QDate(m_date.year(), m_date.month(), day); emit dateSelected(m_selectedDate); update(); }
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

Text* text(const QString& content, Widget* parent) { return Text::create(content, parent); }
Text* title(const QString& content, Widget* parent) { Text* t = Text::create(content, parent); t->setTitle(); return t; }
Text* label(const QString& content, Widget* parent) { return Text::create(content, parent); }
ProgressBar* progressBar(Widget* parent) { return ProgressBar::create(parent); }
Graph* graph(Widget* parent) { return Graph::create(parent); }
Gauge* gauge(Widget* parent) { return Gauge::create(parent); }
Image* image(const QString& path, Widget* parent) { return Image::create(path, parent); }
Button* button(const QString& t, Widget* parent) { return Button::create(t, parent); }
Spacer* spacer(int size, Widget* parent) { return Spacer::create(size, parent); }
Container* container(Container::Layout layout, Widget* parent) { return Container::create(layout, parent); }
Clock* clock(Clock::Style style, Widget* parent) { return Clock::create(style, parent); }
Calendar* calendar(Widget* parent) { return Calendar::create(parent); }

} // namespace Milk
