/**
 * MilkWidgetCore - Widget Implementation
 */

#include "milk/Widget.h"
#include "milk/Parsers.h"
#include "milk/Utils.h"

#include <QPainter>
#include <QPainterPath>
#include <QScreen>
#include <QGuiApplication>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsBlurEffect>
#include <QGraphicsOpacityEffect>
#include <QTimer>
#include <QFile>

#ifdef Q_OS_LINUX
#include <unistd.h>
#ifdef MILK_HAS_X11
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#if QT_VERSION_MAJOR == 5
#include <QX11Info>
#endif
#endif
#endif

namespace Milk {

// ============================================================================
// CONSTRUCTION
// ============================================================================

Widget::Widget(int width, int height, QWidget* parent)
    : QWidget(parent)
{
    resize(width, height);
    setupWidget();
    
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(15, 15, 15, 15);
    m_mainLayout->setSpacing(8);
    
    updateMask();
    m_initialized = true;
}

Widget::~Widget() {
    cleanupAnimations();
    if (m_currentEffect) {
        delete m_currentEffect;
        m_currentEffect = nullptr;
    }
    if (m_updateTimer) {
        m_updateTimer->stop();
        delete m_updateTimer;
    }
}

void Widget::setupWidget() {
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);
    setMouseTracking(true);
    
#ifdef Q_OS_LINUX
    setAttribute(Qt::WA_ShowWithoutActivating);
    setAttribute(Qt::WA_X11NetWmWindowTypeDesktop, false);
#endif
}

// ============================================================================
// STATIC FACTORY METHODS
// ============================================================================

Widget* Widget::create(int width, int height) {
    return new Widget(width, height);
}

Widget* Widget::createCircle(int diameter) {
    Widget* w = new Widget(diameter, diameter);
    w->setShape(Shape::Circle);
    return w;
}

Widget* Widget::createSquare(int size) {
    Widget* w = new Widget(size, size);
    w->setShape(Shape::Square);
    return w;
}

Widget* Widget::fromFile(const QString& xmlPath) {
    XMLParser parser;
    QList<Widget*> widgets = parser.parseFile(xmlPath);
    return widgets.isEmpty() ? nullptr : widgets.first();
}

Widget* Widget::fromString(const QString& xml) {
    XMLParser parser;
    QList<Widget*> widgets = parser.parseString(xml);
    return widgets.isEmpty() ? nullptr : widgets.first();
}

// ============================================================================
// SHAPE & GEOMETRY
// ============================================================================

void Widget::setShape(Shape shape) {
    m_shape = shape;
    if (shape == Shape::Circle || shape == Shape::Square) {
        int s = qMin(width(), height());
        resize(s, s);
    }
    updateMask();
    update();
}

void Widget::setRounded(int radius) {
    m_cornerRadius = radius;
    m_shape = Shape::RoundedRect;
    updateMask();
    update();
}

void Widget::setSize(int w, int h) {
    resize(w, h);
    updateMask();
    update();
}

void Widget::setMinSize(int w, int h) {
    setMinimumSize(w, h);
}

void Widget::setMaxSize(int w, int h) {
    setMaximumSize(w, h);
}

void Widget::updateMask() {
    QPainterPath path;
    QRectF rect = this->rect();
    
    switch (m_shape) {
        case Shape::Rectangle:
            path.addRect(rect);
            break;
        case Shape::RoundedRect:
            path.addRoundedRect(rect, m_cornerRadius, m_cornerRadius);
            break;
        case Shape::Circle: {
            qreal diameter = qMin(rect.width(), rect.height());
            qreal radius = diameter / 2.0;
            QPointF center = rect.center();
            path.addEllipse(center.x() - radius, center.y() - radius, diameter, diameter);
            break;
        }
        case Shape::Ellipse:
            path.addEllipse(rect);
            break;
        case Shape::Square:
            path.addRect(rect);
            break;
        case Shape::Custom:
            path.addRect(rect);
            break;
    }
    
    QRegion region = QRegion(path.toFillPolygon().toPolygon());
    setMask(region);
}

// ============================================================================
// BACKGROUND & APPEARANCE
// ============================================================================

void Widget::setBackground(const QString& color) {
    m_bgColor = Color::parse(color);
    update();
}

void Widget::setBackground(const QColor& color) {
    m_bgColor = color;
    update();
}

void Widget::setBackground(int r, int g, int b, int a) {
    m_bgColor = QColor(r, g, b, a);
    update();
}

void Widget::setBackgroundGradient(const QColor& start, const QColor& end, double angle) {
    m_bgGradient.type = Gradient::Linear;
    m_bgGradient.start = start;
    m_bgGradient.end = end;
    m_bgGradient.angle = angle;
    update();
}

void Widget::setBackgroundImage(const QString& path) {
    m_bgImage = path;
    update();
}

void Widget::setOpacity(double opacity) {
    m_opacity = qBound(0.0, opacity, 1.0);
    setWindowOpacity(m_opacity);
}

// ============================================================================
// EFFECTS
// ============================================================================

void Widget::setBlur(BlurMode mode, double radius) {
    m_blurMode = mode;
    m_blurRadius = radius;
    
    if (m_currentEffect) {
        delete m_currentEffect;
        m_currentEffect = nullptr;
        setGraphicsEffect(nullptr);
    }
    
    if (mode != BlurMode::None) {
        // Use shadow effect to simulate glass
        auto* shadow = new QGraphicsDropShadowEffect(this);
        shadow->setBlurRadius(15);
        shadow->setColor(QColor(0, 0, 0, 60));
        shadow->setOffset(0, 3);
        m_currentEffect = shadow;
        setGraphicsEffect(m_currentEffect);
        
        // Make background more transparent for glass effect
        if (m_bgColor.alpha() > 150) {
            m_bgColor.setAlpha(100);
        }
    }
}

void Widget::setGlass(bool enabled) {
    setBlur(enabled ? BlurMode::Glass : BlurMode::None, 10.0);
}

void Widget::setShadow(const QColor& color, int blur, int offsetX, int offsetY) {
    m_shadow.color = color;
    m_shadow.blur = blur;
    m_shadow.offsetX = offsetX;
    m_shadow.offsetY = offsetY;
    m_shadow.enabled = true;
    
    if (m_currentEffect) {
        delete m_currentEffect;
        m_currentEffect = nullptr;
    }
    
    auto* effect = new QGraphicsDropShadowEffect(this);
    effect->setBlurRadius(blur);
    effect->setColor(color);
    effect->setOffset(offsetX, offsetY);
    m_currentEffect = effect;
    setGraphicsEffect(effect);
}

void Widget::setShadow(const Shadow& shadow) {
    setShadow(shadow.color, shadow.blur, shadow.offsetX, shadow.offsetY);
}

void Widget::removeShadow() {
    m_shadow.enabled = false;
    if (m_currentEffect) {
        delete m_currentEffect;
        m_currentEffect = nullptr;
        setGraphicsEffect(nullptr);
    }
}

void Widget::setGlow(const QString& color, int intensity) {
    setGlow(Color::parse(color), intensity);
}

void Widget::setGlow(const QColor& color, int intensity) {
    if (m_currentEffect) {
        delete m_currentEffect;
        m_currentEffect = nullptr;
    }
    
    auto* effect = new QGraphicsDropShadowEffect(this);
    effect->setBlurRadius(qBound(5, intensity * 2, 50));
    effect->setColor(color);
    effect->setOffset(0, 0);
    m_currentEffect = effect;
    setGraphicsEffect(effect);
}

void Widget::removeGlow() {
    removeShadow();
}

// ============================================================================
// BORDER
// ============================================================================

void Widget::setBorder(const QString& color, int width) {
    setBorder(Color::parse(color), width);
}

void Widget::setBorder(const QColor& color, int width) {
    m_border.color = color;
    m_border.width = width;
    update();
}

void Widget::setBorder(const Border& border) {
    m_border = border;
    update();
}

void Widget::removeBorder() {
    m_border.width = 0;
    update();
}

// ============================================================================
// POSITIONING
// ============================================================================

void Widget::setPosition(Position pos) {
    m_position = pos;
    updatePosition();
}

void Widget::setPosition(int x, int y) {
    m_position = Position::Manual;
    move(x, y);
}

void Widget::setScreenMargin(int margin) {
    m_screenMargin = margin;
    if (m_position != Position::Manual) {
        updatePosition();
    }
}

void Widget::updatePosition() {
    QPoint pos = Screen::calculatePosition(m_position, size(), m_screenMargin);
    move(pos);
}

void Widget::center() {
    setPosition(Position::Center);
}

void Widget::toFront() {
    raise();
    activateWindow();
}

void Widget::toBack() {
    lower();
}

// ============================================================================
// WINDOW BEHAVIOR
// ============================================================================

void Widget::setWindowType(WindowType type) {
    m_windowType = type;
    applyWindowFlags();
}

void Widget::applyWindowFlags() {
    Qt::WindowFlags flags = Qt::FramelessWindowHint | Qt::Tool;
    
    switch (m_windowType) {
        case WindowType::Normal:
            flags |= Qt::WindowStaysOnTopHint;
            break;
        case WindowType::Desktop:
            flags |= Qt::WindowStaysOnBottomHint;
            break;
        case WindowType::Dock:
            flags |= Qt::WindowStaysOnTopHint;
            break;
        case WindowType::Notification:
            flags |= Qt::WindowStaysOnTopHint | Qt::WindowDoesNotAcceptFocus;
            break;
        case WindowType::Overlay:
            flags |= Qt::WindowStaysOnTopHint | Qt::WindowTransparentForInput;
            break;
    }
    
    setWindowFlags(flags);
    if (isVisible()) show();
}

void Widget::setDraggable(bool enabled) {
    m_draggable = enabled;
}

void Widget::setClickThrough(bool enabled) {
    setAttribute(Qt::WA_TransparentForMouseEvents, enabled);
}

void Widget::setAlwaysOnTop(bool enabled) {
    Qt::WindowFlags flags = windowFlags();
    if (enabled) {
        flags |= Qt::WindowStaysOnTopHint;
        flags &= ~Qt::WindowStaysOnBottomHint;
    } else {
        flags &= ~Qt::WindowStaysOnTopHint;
    }
    setWindowFlags(flags);
    if (isVisible()) show();
}

void Widget::setSticky(bool enabled) {
    Q_UNUSED(enabled)
    // Platform-specific implementation
#ifdef Q_OS_LINUX
    applyX11Properties();
#endif
}

void Widget::setSkipTaskbar(bool enabled) {
    Qt::WindowFlags flags = windowFlags();
    if (enabled) {
        flags |= Qt::Tool;
    }
    setWindowFlags(flags);
    if (isVisible()) show();
}

void Widget::setSkipPager(bool enabled) {
    Q_UNUSED(enabled)
#ifdef Q_OS_LINUX
    applyX11Properties();
#endif
}

void Widget::applyX11Properties() {
#if defined(Q_OS_LINUX) && defined(MILK_HAS_X11)
    // X11-specific window properties would go here
    // Using EWMH hints for sticky, skip taskbar/pager, etc.
#endif
}

// ============================================================================
// ANIMATIONS
// ============================================================================

QEasingCurve::Type Widget::toQtEasing(Easing e) {
    switch (e) {
        case Easing::Linear: return QEasingCurve::Linear;
        case Easing::InQuad: return QEasingCurve::InQuad;
        case Easing::OutQuad: return QEasingCurve::OutQuad;
        case Easing::InOutQuad: return QEasingCurve::InOutQuad;
        case Easing::InCubic: return QEasingCurve::InCubic;
        case Easing::OutCubic: return QEasingCurve::OutCubic;
        case Easing::InOutCubic: return QEasingCurve::InOutCubic;
        case Easing::InElastic: return QEasingCurve::InElastic;
        case Easing::OutElastic: return QEasingCurve::OutElastic;
        case Easing::InOutElastic: return QEasingCurve::InOutElastic;
        case Easing::InBounce: return QEasingCurve::InBounce;
        case Easing::OutBounce: return QEasingCurve::OutBounce;
        case Easing::InOutBounce: return QEasingCurve::InOutBounce;
        default: return QEasingCurve::OutCubic;
    }
}

void Widget::cleanupAnimations() {
    for (auto& anim : m_animations) {
        if (anim) {
            anim->stop();
            anim->deleteLater();
        }
    }
    m_animations.clear();
}

void Widget::stopAnimation(const QString& name) {
    if (m_animations.contains(name) && m_animations[name]) {
        m_animations[name]->stop();
        m_animations[name]->deleteLater();
        m_animations.remove(name);
    }
}

QPropertyAnimation* Widget::createAnimation(const QByteArray& property, int duration) {
    auto* anim = new QPropertyAnimation(this, property, this);
    anim->setDuration(duration);
    return anim;
}

void Widget::fadeIn(int duration, Easing easing) {
    if (!m_initialized) return;
    
    stopAnimation("fade");
    
    auto* anim = createAnimation("windowOpacity", duration);
    anim->setStartValue(0.0);
    anim->setEndValue(m_opacity);
    anim->setEasingCurve(toQtEasing(easing));
    
    connect(anim, &QPropertyAnimation::finished, [this]() {
        m_animations.remove("fade");
        emit animationFinished(Animation::FadeIn);
        if (m_animationCallback) m_animationCallback();
    });
    
    m_animations["fade"] = anim;
    anim->start();
}

void Widget::fadeOut(int duration, Easing easing) {
    if (!m_initialized) return;
    
    stopAnimation("fade");
    
    auto* anim = createAnimation("windowOpacity", duration);
    anim->setStartValue(windowOpacity());
    anim->setEndValue(0.0);
    anim->setEasingCurve(toQtEasing(easing));
    
    connect(anim, &QPropertyAnimation::finished, [this]() {
        QWidget::hide();
        m_animations.remove("fade");
        emit animationFinished(Animation::FadeOut);
        if (m_animationCallback) m_animationCallback();
    });
    
    m_animations["fade"] = anim;
    anim->start();
}

void Widget::fadeTo(double opacity, int duration, Easing easing) {
    if (!m_initialized) return;
    
    stopAnimation("fade");
    
    auto* anim = createAnimation("windowOpacity", duration);
    anim->setStartValue(windowOpacity());
    anim->setEndValue(opacity);
    anim->setEasingCurve(toQtEasing(easing));
    
    connect(anim, &QPropertyAnimation::finished, [this]() {
        m_animations.remove("fade");
    });
    
    m_animations["fade"] = anim;
    anim->start();
}

void Widget::bounce(int duration) {
    if (!m_initialized) return;
    
    stopAnimation("bounce");
    
    auto* anim = createAnimation("geometry", duration);
    QRect current = geometry();
    QRect bounced = current;
    bounced.setSize(QSize(current.width() * 1.05, current.height() * 1.05));
    bounced.moveCenter(current.center());
    
    anim->setStartValue(current);
    anim->setKeyValueAt(0.5, bounced);
    anim->setEndValue(current);
    anim->setEasingCurve(QEasingCurve::OutBounce);
    
    connect(anim, &QPropertyAnimation::finished, [this]() {
        m_animations.remove("bounce");
        emit animationFinished(Animation::Bounce);
    });
    
    m_animations["bounce"] = anim;
    anim->start();
}

void Widget::pulse(int duration) {
    if (!m_initialized) return;
    
    stopAnimation("pulse");
    
    auto* anim = createAnimation("windowOpacity", duration);
    anim->setStartValue(m_opacity);
    anim->setKeyValueAt(0.5, m_opacity * 0.7);
    anim->setEndValue(m_opacity);
    anim->setEasingCurve(QEasingCurve::InOutSine);
    anim->setLoopCount(-1);  // Infinite
    
    m_animations["pulse"] = anim;
    anim->start();
}

void Widget::shake(int duration, int intensity) {
    if (!m_initialized) return;
    
    stopAnimation("shake");
    
    auto* anim = createAnimation("pos", duration);
    QPoint original = pos();
    
    anim->setStartValue(original);
    anim->setKeyValueAt(0.1, original + QPoint(intensity, 0));
    anim->setKeyValueAt(0.2, original + QPoint(-intensity, 0));
    anim->setKeyValueAt(0.3, original + QPoint(intensity, 0));
    anim->setKeyValueAt(0.4, original + QPoint(-intensity, 0));
    anim->setKeyValueAt(0.5, original + QPoint(intensity, 0));
    anim->setKeyValueAt(0.6, original + QPoint(-intensity, 0));
    anim->setKeyValueAt(0.7, original + QPoint(intensity, 0));
    anim->setKeyValueAt(0.8, original + QPoint(-intensity, 0));
    anim->setEndValue(original);
    anim->setEasingCurve(QEasingCurve::OutQuad);
    
    connect(anim, &QPropertyAnimation::finished, [this]() {
        m_animations.remove("shake");
        emit animationFinished(Animation::Shake);
    });
    
    m_animations["shake"] = anim;
    anim->start();
}

void Widget::scaleTo(double scale, int duration) {
    if (!m_initialized) return;
    
    stopAnimation("scale");
    
    QRect current = geometry();
    QRect target = current;
    int newW = static_cast<int>(current.width() * scale);
    int newH = static_cast<int>(current.height() * scale);
    target.setSize(QSize(newW, newH));
    target.moveCenter(current.center());
    
    auto* anim = createAnimation("geometry", duration);
    anim->setStartValue(current);
    anim->setEndValue(target);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    
    connect(anim, &QPropertyAnimation::finished, [this]() {
        m_animations.remove("scale");
        emit animationFinished(Animation::Scale);
    });
    
    m_animations["scale"] = anim;
    anim->start();
}

void Widget::moveTo(int x, int y, int duration, Easing easing) {
    if (!m_initialized) return;
    
    stopAnimation("move");
    
    auto* anim = createAnimation("pos", duration);
    anim->setStartValue(pos());
    anim->setEndValue(QPoint(x, y));
    anim->setEasingCurve(toQtEasing(easing));
    
    connect(anim, &QPropertyAnimation::finished, [this]() {
        m_animations.remove("move");
    });
    
    m_animations["move"] = anim;
    anim->start();
}

void Widget::slideIn(Position from, int duration) {
    if (!m_initialized) return;
    
    QPoint target = Screen::calculatePosition(m_position, size(), m_screenMargin);
    QPoint start = target;
    QSize screen = Screen::size();
    
    switch (from) {
        case Position::TopLeft:
        case Position::TopCenter:
        case Position::TopRight:
            start.setY(-height());
            break;
        case Position::BottomLeft:
        case Position::BottomCenter:
        case Position::BottomRight:
            start.setY(screen.height());
            break;
        case Position::CenterLeft:
            start.setX(-width());
            break;
        case Position::CenterRight:
            start.setX(screen.width());
            break;
        default:
            break;
    }
    
    move(start);
    QWidget::show();
    
    stopAnimation("slide");
    
    auto* anim = createAnimation("pos", duration);
    anim->setStartValue(start);
    anim->setEndValue(target);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    
    connect(anim, &QPropertyAnimation::finished, [this]() {
        m_animations.remove("slide");
        emit animationFinished(Animation::SlideIn);
    });
    
    m_animations["slide"] = anim;
    anim->start();
}

void Widget::slideOut(Position to, int duration) {
    if (!m_initialized) return;
    
    QPoint start = pos();
    QPoint target = start;
    QSize screen = Screen::size();
    
    switch (to) {
        case Position::TopLeft:
        case Position::TopCenter:
        case Position::TopRight:
            target.setY(-height());
            break;
        case Position::BottomLeft:
        case Position::BottomCenter:
        case Position::BottomRight:
            target.setY(screen.height());
            break;
        case Position::CenterLeft:
            target.setX(-width());
            break;
        case Position::CenterRight:
            target.setX(screen.width());
            break;
        default:
            break;
    }
    
    stopAnimation("slide");
    
    auto* anim = createAnimation("pos", duration);
    anim->setStartValue(start);
    anim->setEndValue(target);
    anim->setEasingCurve(QEasingCurve::InCubic);
    
    connect(anim, &QPropertyAnimation::finished, [this]() {
        QWidget::hide();
        m_animations.remove("slide");
        emit animationFinished(Animation::SlideOut);
    });
    
    m_animations["slide"] = anim;
    anim->start();
}

void Widget::stopAnimations() {
    cleanupAnimations();
}

bool Widget::isAnimating() const {
    return !m_animations.isEmpty();
}

void Widget::setAnimationCallback(AnimationCallback callback) {
    m_animationCallback = callback;
}

// ============================================================================
// LAYOUT
// ============================================================================

QVBoxLayout* Widget::vbox() {
    return m_mainLayout;
}

QHBoxLayout* Widget::hbox() {
    auto* layout = new QHBoxLayout();
    m_mainLayout->addLayout(layout);
    return layout;
}

QGridLayout* Widget::grid() {
    auto* layout = new QGridLayout();
    m_mainLayout->addLayout(layout);
    return layout;
}

void Widget::addWidget(QWidget* widget) {
    m_mainLayout->addWidget(widget);
}

void Widget::addSpacing(int size) {
    m_mainLayout->addSpacing(size);
}

void Widget::addStretch(int factor) {
    m_mainLayout->addStretch(factor);
}

void Widget::setMargin(int margin) {
    m_mainLayout->setContentsMargins(margin, margin, margin, margin);
}

void Widget::setMargin(int top, int right, int bottom, int left) {
    m_mainLayout->setContentsMargins(left, top, right, bottom);
}

void Widget::setPadding(int padding) {
    setMargin(padding);
}

void Widget::setSpacing(int spacing) {
    m_mainLayout->setSpacing(spacing);
}

// ============================================================================
// STYLE
// ============================================================================

void Widget::setStyle(const StyleSheet& style) {
    m_style = style;
    
    if (style.backgroundColor.isValid()) {
        m_bgColor = style.backgroundColor;
    }
    if (style.backgroundGradient.isValid()) {
        m_bgGradient = style.backgroundGradient;
    }
    if (style.cornerRadius > 0) {
        setRounded(style.cornerRadius);
    }
    if (style.border.isVisible()) {
        setBorder(style.border);
    }
    if (style.shadow.enabled) {
        setShadow(style.shadow);
    }
    if (style.opacity < 1.0) {
        setOpacity(style.opacity);
    }
    if (style.blur != BlurMode::None) {
        setBlur(style.blur, style.blurRadius);
    }
    
    m_mainLayout->setContentsMargins(
        style.padding.left + style.margin.left,
        style.padding.top + style.margin.top,
        style.padding.right + style.margin.right,
        style.padding.bottom + style.margin.bottom
    );
    
    update();
}

void Widget::setStyleClass(const QString& className) {
    // Would get style from global CSS parser
    Q_UNUSED(className)
}

void Widget::loadStyleSheet(const QString& cssPath) {
    Q_UNUSED(cssPath)
}

void Widget::applyCSS(const QString& css) {
    Q_UNUSED(css)
}

// ============================================================================
// EVENTS & CALLBACKS
// ============================================================================

void Widget::onClick(ClickCallback callback) {
    m_onClick = callback;
}

void Widget::onHover(HoverCallback callback) {
    m_onHover = callback;
}

void Widget::onUpdate(UpdateCallback callback) {
    m_onUpdate = callback;
    
    if (!m_updateTimer) {
        m_updateTimer = new QTimer(this);
        connect(m_updateTimer, &QTimer::timeout, [this]() {
            if (m_onUpdate) m_onUpdate();
        });
    }
    
    if (!m_updateTimer->isActive()) {
        m_updateTimer->start(1000);
    }
}

void Widget::setUpdateInterval(int ms) {
    if (m_updateTimer) {
        m_updateTimer->setInterval(ms);
    }
}

// ============================================================================
// SERIALIZATION
// ============================================================================

QString Widget::toXml() const {
    QString xml = "<widget";
    xml += QString(" width=\"%1\"").arg(width());
    xml += QString(" height=\"%1\"").arg(height());
    xml += QString(" background=\"%1\"").arg(Color::toString(m_bgColor));
    if (m_cornerRadius > 0) {
        xml += QString(" rounded=\"%1\"").arg(m_cornerRadius);
    }
    xml += ">\n";
    
    // Children would be serialized here
    
    xml += "</widget>";
    return xml;
}

QString Widget::toCSS() const {
    QString css = ".widget {\n";
    css += QString("  background-color: %1;\n").arg(Color::toString(m_bgColor));
    css += QString("  width: %1px;\n").arg(width());
    css += QString("  height: %1px;\n").arg(height());
    if (m_cornerRadius > 0) {
        css += QString("  border-radius: %1px;\n").arg(m_cornerRadius);
    }
    if (m_border.isVisible()) {
        css += QString("  border: %1px solid %2;\n")
            .arg(m_border.width)
            .arg(Color::toString(m_border.color));
    }
    css += "}\n";
    return css;
}

void Widget::saveToFile(const QString& path) const {
    QFile file(path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << toXml();
    }
}

// ============================================================================
// VISIBILITY
// ============================================================================

void Widget::show() {
    QWidget::show();
    if (m_initialized) {
        fadeIn(200);
    }
    emit shown();
}

void Widget::hide() {
    if (m_initialized) {
        fadeOut(200);
    } else {
        QWidget::hide();
    }
    emit hidden();
}

void Widget::toggle() {
    if (isVisible()) {
        hide();
    } else {
        show();
    }
}

// ============================================================================
// PAINT EVENT
// ============================================================================

void Widget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event)
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    
    QPainterPath path;
    QRectF rect = this->rect();
    
    switch (m_shape) {
        case Shape::Rectangle:
            path.addRect(rect);
            break;
        case Shape::RoundedRect:
            path.addRoundedRect(rect, m_cornerRadius, m_cornerRadius);
            break;
        case Shape::Circle: {
            qreal diameter = qMin(rect.width(), rect.height());
            qreal radius = diameter / 2.0;
            QPointF center = rect.center();
            path.addEllipse(center.x() - radius, center.y() - radius, diameter, diameter);
            break;
        }
        case Shape::Ellipse:
            path.addEllipse(rect);
            break;
        case Shape::Square:
            path.addRect(rect);
            break;
        case Shape::Custom:
            path.addRect(rect);
            break;
    }
    
    // Background
    if (m_bgGradient.isValid()) {
        QLinearGradient gradient;
        gradient.setStart(0, 0);
        gradient.setFinalStop(rect.width(), rect.height());
        gradient.setColorAt(0, m_bgGradient.start);
        gradient.setColorAt(1, m_bgGradient.end);
        painter.fillPath(path, gradient);
    } else {
        painter.fillPath(path, m_bgColor);
    }
    
    // Background image
    if (!m_bgImage.isEmpty()) {
        QPixmap pixmap(m_bgImage);
        if (!pixmap.isNull()) {
            painter.setClipPath(path);
            painter.drawPixmap(rect.toRect(), pixmap);
            painter.setClipping(false);
        }
    }
    
    // Border
    if (m_border.isVisible()) {
        QPen pen(m_border.color, m_border.width);
        pen.setJoinStyle(Qt::RoundJoin);
        pen.setCapStyle(Qt::RoundCap);
        painter.setPen(pen);
        painter.drawPath(path);
    }
}

// ============================================================================
// MOUSE EVENTS
// ============================================================================

void Widget::mousePressEvent(QMouseEvent* event) {
    if (m_draggable && event->button() == Qt::LeftButton) {
        m_dragPos = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
    }
    QWidget::mousePressEvent(event);
}

void Widget::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        if (m_onClick) {
            m_onClick();
        }
        emit clicked();
    }
    QWidget::mouseReleaseEvent(event);
}

void Widget::mouseMoveEvent(QMouseEvent* event) {
    if (m_draggable && (event->buttons() & Qt::LeftButton)) {
        move(event->globalPosition().toPoint() - m_dragPos);
        emit positionChanged(x(), y());
        event->accept();
    }
    QWidget::mouseMoveEvent(event);
}

void Widget::enterEvent(QEnterEvent* event) {
    if (m_onHover) m_onHover(true);
    emit hovered(true);
    QWidget::enterEvent(event);
}

void Widget::leaveEvent(QEvent* event) {
    if (m_onHover) m_onHover(false);
    emit hovered(false);
    QWidget::leaveEvent(event);
}

void Widget::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    updateMask();
}

void Widget::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    m_initialized = true;
}

void Widget::hideEvent(QHideEvent* event) {
    QWidget::hideEvent(event);
}

} // namespace Milk
