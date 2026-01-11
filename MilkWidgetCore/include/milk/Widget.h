/**
 * MilkWidgetCore - Main Widget Class
 */

#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPropertyAnimation>
#include <QGraphicsEffect>
#include <QPointer>
#include <QMap>
#include <memory>

#include "Types.h"

namespace Milk {

class Widget : public QWidget {
    Q_OBJECT
    Q_PROPERTY(double opacity READ windowOpacity WRITE setWindowOpacity)
    
public:
    // Construction
    explicit Widget(int width = 300, int height = 200, QWidget* parent = nullptr);
    virtual ~Widget();
    
    // Static factory methods
    static Widget* create(int width = 300, int height = 200);
    static Widget* createCircle(int diameter);
    static Widget* createSquare(int size);
    static Widget* fromFile(const QString& xmlPath);
    static Widget* fromString(const QString& xml);
    
    // ========================================================================
    // Shape & Geometry
    // ========================================================================
    void setShape(Shape shape);
    Shape shape() const { return m_shape; }
    
    void setRounded(int radius);
    int cornerRadius() const { return m_cornerRadius; }
    
    void setSize(int width, int height);
    void setMinSize(int width, int height);
    void setMaxSize(int width, int height);
    
    // ========================================================================
    // Background & Appearance
    // ========================================================================
    void setBackground(const QString& color);
    void setBackground(const QColor& color);
    void setBackground(int r, int g, int b, int a = 255);
    void setBackgroundGradient(const QColor& start, const QColor& end, double angle = 0);
    void setBackgroundImage(const QString& path);
    QColor backgroundColor() const { return m_bgColor; }
    
    void setOpacity(double opacity);
    double opacity() const { return m_opacity; }
    
    // ========================================================================
    // Effects
    // ========================================================================
    void setBlur(BlurMode mode, double radius = 10.0);
    void setGlass(bool enabled = true);
    void setShadow(const QColor& color, int blur = 10, int offsetX = 0, int offsetY = 2);
    void setShadow(const Shadow& shadow);
    void removeShadow();
    void setGlow(const QString& color, int intensity = 10);
    void setGlow(const QColor& color, int intensity = 10);
    void removeGlow();
    
    // ========================================================================
    // Border
    // ========================================================================
    void setBorder(const QString& color, int width = 1);
    void setBorder(const QColor& color, int width = 1);
    void setBorder(const Border& border);
    void removeBorder();
    Border border() const { return m_border; }
    
    // ========================================================================
    // Positioning
    // ========================================================================
    void setPosition(Position pos);
    void setPosition(int x, int y);
    void setScreenMargin(int margin);
    void center();
    void toFront();
    void toBack();
    
    Position position() const { return m_position; }
    
    // ========================================================================
    // Window Behavior
    // ========================================================================
    void setWindowType(WindowType type);
    void setDraggable(bool enabled = true);
    void setClickThrough(bool enabled = true);
    void setAlwaysOnTop(bool enabled = true);
    void setSticky(bool enabled = true);  // Visible on all workspaces
    void setSkipTaskbar(bool enabled = true);
    void setSkipPager(bool enabled = true);
    
    bool isDraggable() const { return m_draggable; }
    
    // ========================================================================
    // Animations
    // ========================================================================
    void fadeIn(int duration = 300, Easing easing = Easing::OutCubic);
    void fadeOut(int duration = 300, Easing easing = Easing::OutCubic);
    void fadeTo(double opacity, int duration = 300, Easing easing = Easing::OutCubic);
    void bounce(int duration = 500);
    void pulse(int duration = 1000);
    void shake(int duration = 500, int intensity = 10);
    void scaleTo(double scale, int duration = 300);
    void moveTo(int x, int y, int duration = 500, Easing easing = Easing::OutCubic);
    void slideIn(Position from, int duration = 300);
    void slideOut(Position to, int duration = 300);
    
    void stopAnimations();
    bool isAnimating() const;
    
    void setAnimationCallback(AnimationCallback callback);
    
    // ========================================================================
    // Layout
    // ========================================================================
    QVBoxLayout* vbox();
    QHBoxLayout* hbox();
    QGridLayout* grid();
    void addWidget(QWidget* widget);
    void addSpacing(int size);
    void addStretch(int factor = 1);
    
    void setMargin(int margin);
    void setMargin(int top, int right, int bottom, int left);
    void setPadding(int padding);
    void setSpacing(int spacing);
    
    // ========================================================================
    // Style
    // ========================================================================
    void setStyle(const StyleSheet& style);
    void setStyleClass(const QString& className);
    void loadStyleSheet(const QString& cssPath);
    void applyCSS(const QString& css);
    
    StyleSheet styleSheet() const { return m_style; }
    
    // ========================================================================
    // Events & Callbacks
    // ========================================================================
    void onClick(ClickCallback callback);
    void onHover(HoverCallback callback);
    void onUpdate(UpdateCallback callback);
    void setUpdateInterval(int ms);
    
    // ========================================================================
    // Serialization
    // ========================================================================
    QString toXml() const;
    QString toCSS() const;
    void saveToFile(const QString& path) const;
    
    // ========================================================================
    // Visibility
    // ========================================================================
    void show();
    void hide();
    void toggle();
    
signals:
    void clicked();
    void hovered(bool enter);
    void shown();
    void hidden();
    void positionChanged(int x, int y);
    void animationFinished(Animation type);
    
protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    
private:
    void setupWidget();
    void applyWindowFlags();
    void applyX11Properties();
    void updateMask();
    void updatePosition();
    QEasingCurve::Type toQtEasing(Easing e);
    
    void cleanupAnimations();
    void stopAnimation(const QString& name);
    QPropertyAnimation* createAnimation(const QByteArray& property, int duration);
    
private:
    // Layout
    QVBoxLayout* m_mainLayout;
    
    // Shape & Geometry
    Shape m_shape = Shape::Rectangle;
    int m_cornerRadius = 0;
    Position m_position = Position::Center;
    int m_screenMargin = 50;
    
    // Appearance
    QColor m_bgColor = QColor(30, 30, 40, 220);
    Gradient m_bgGradient;
    QString m_bgImage;
    double m_opacity = 1.0;
    Border m_border;
    Shadow m_shadow;
    StyleSheet m_style;
    
    // Effects
    BlurMode m_blurMode = BlurMode::None;
    double m_blurRadius = 10.0;
    QGraphicsEffect* m_currentEffect = nullptr;
    
    // Behavior
    bool m_draggable = true;
    bool m_initialized = false;
    QPoint m_dragPos;
    WindowType m_windowType = WindowType::Normal;
    
    // Animations
    QMap<QString, QPointer<QPropertyAnimation>> m_animations;
    AnimationCallback m_animationCallback;
    
    // Callbacks
    ClickCallback m_onClick;
    HoverCallback m_onHover;
    UpdateCallback m_onUpdate;
    QTimer* m_updateTimer = nullptr;
};

} // namespace Milk
