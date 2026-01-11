/**
 * MilkWidgetCore - Utilities Implementation
 */

#include "milk/Utils.h"
#include "milk/Types.h"

#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QTextStream>
#include <QStandardPaths>
#include <QScreen>
#include <QGuiApplication>
#include <QRandomGenerator>
#include <QTimer>
#include <cmath>

namespace Milk {

// ============================================================================
// COLOR UTILITIES
// ============================================================================

namespace Color {

// Predefined colors
const QColor Transparent = Qt::transparent;
const QColor White = Qt::white;
const QColor Black = Qt::black;
const QColor Red = QColor(255, 0, 0);
const QColor Green = QColor(0, 255, 0);
const QColor Blue = QColor(0, 0, 255);
const QColor Yellow = QColor(255, 255, 0);
const QColor Cyan = QColor(0, 255, 255);
const QColor Magenta = QColor(255, 0, 255);
const QColor Orange = QColor(255, 165, 0);
const QColor Purple = QColor(128, 0, 128);
const QColor Pink = QColor(255, 192, 203);
const QColor Gray = QColor(128, 128, 128);
const QColor DarkGray = QColor(64, 64, 64);
const QColor LightGray = QColor(192, 192, 192);

QColor parse(const QString& str) {
    QString s = str.trimmed().toLower();
    
    // Check for rgb/rgba format
    if (s.startsWith("rgb(")) {
        QString inner = s.mid(4, s.length() - 5);
        QStringList parts = inner.split(',');
        if (parts.size() >= 3) {
            return QColor(
                parts[0].trimmed().toInt(),
                parts[1].trimmed().toInt(),
                parts[2].trimmed().toInt()
            );
        }
    }
    
    if (s.startsWith("rgba(")) {
        QString inner = s.mid(5, s.length() - 6);
        QStringList parts = inner.split(',');
        if (parts.size() >= 4) {
            int alpha = 255;
            QString alphaStr = parts[3].trimmed();
            if (alphaStr.contains('.')) {
                alpha = static_cast<int>(alphaStr.toDouble() * 255);
            } else {
                alpha = alphaStr.toInt();
            }
            return QColor(
                parts[0].trimmed().toInt(),
                parts[1].trimmed().toInt(),
                parts[2].trimmed().toInt(),
                alpha
            );
        }
    }
    
    // Check for hsl/hsla format
    if (s.startsWith("hsl(")) {
        QString inner = s.mid(4, s.length() - 5);
        QStringList parts = inner.split(',');
        if (parts.size() >= 3) {
            int h = parts[0].trimmed().toInt();
            int sat = parts[1].trimmed().remove('%').toInt();
            int light = parts[2].trimmed().remove('%').toInt();
            QColor c;
            c.setHsl(h, sat * 255 / 100, light * 255 / 100);
            return c;
        }
    }
    
    // Named colors
    if (s == "transparent") return Qt::transparent;
    if (s == "white") return Qt::white;
    if (s == "black") return Qt::black;
    if (s == "red") return Red;
    if (s == "green") return Green;
    if (s == "blue") return Blue;
    if (s == "yellow") return Yellow;
    if (s == "cyan") return Cyan;
    if (s == "magenta") return Magenta;
    if (s == "orange") return Orange;
    if (s == "purple") return Purple;
    if (s == "pink") return Pink;
    if (s == "gray" || s == "grey") return Gray;
    
    // Hex format (default)
    return QColor(str);
}

QString toString(const QColor& color, bool includeAlpha) {
    if (!color.isValid()) return "transparent";
    
    if (includeAlpha && color.alpha() < 255) {
        return toRgba(color);
    }
    return toHex(color);
}

QString toHex(const QColor& color, bool includeAlpha) {
    if (includeAlpha && color.alpha() < 255) {
        return QString("#%1%2%3%4")
            .arg(color.red(), 2, 16, QChar('0'))
            .arg(color.green(), 2, 16, QChar('0'))
            .arg(color.blue(), 2, 16, QChar('0'))
            .arg(color.alpha(), 2, 16, QChar('0'));
    }
    return color.name();
}

QString toRgb(const QColor& color) {
    return QString("rgb(%1, %2, %3)")
        .arg(color.red())
        .arg(color.green())
        .arg(color.blue());
}

QString toRgba(const QColor& color) {
    return QString("rgba(%1, %2, %3, %4)")
        .arg(color.red())
        .arg(color.green())
        .arg(color.blue())
        .arg(color.alpha());
}

QString toHsl(const QColor& color) {
    int h, s, l;
    color.getHsl(&h, &s, &l);
    return QString("hsl(%1, %2%, %3%)")
        .arg(h)
        .arg(s * 100 / 255)
        .arg(l * 100 / 255);
}

QColor lighten(const QColor& color, double amount) {
    int h, s, l, a;
    color.getHsl(&h, &s, &l, &a);
    l = qBound(0, static_cast<int>(l + 255 * amount), 255);
    QColor result;
    result.setHsl(h, s, l, a);
    return result;
}

QColor darken(const QColor& color, double amount) {
    return lighten(color, -amount);
}

QColor saturate(const QColor& color, double amount) {
    int h, s, l, a;
    color.getHsl(&h, &s, &l, &a);
    s = qBound(0, static_cast<int>(s + 255 * amount), 255);
    QColor result;
    result.setHsl(h, s, l, a);
    return result;
}

QColor desaturate(const QColor& color, double amount) {
    return saturate(color, -amount);
}

QColor adjustAlpha(const QColor& color, double alpha) {
    QColor result = color;
    result.setAlphaF(qBound(0.0, alpha, 1.0));
    return result;
}

QColor withAlpha(const QColor& color, int alpha) {
    QColor result = color;
    result.setAlpha(qBound(0, alpha, 255));
    return result;
}

QColor mix(const QColor& a, const QColor& b, double ratio) {
    ratio = qBound(0.0, ratio, 1.0);
    return QColor(
        static_cast<int>(a.red() * (1 - ratio) + b.red() * ratio),
        static_cast<int>(a.green() * (1 - ratio) + b.green() * ratio),
        static_cast<int>(a.blue() * (1 - ratio) + b.blue() * ratio),
        static_cast<int>(a.alpha() * (1 - ratio) + b.alpha() * ratio)
    );
}

QColor overlay(const QColor& base, const QColor& blend) {
    double alpha = blend.alphaF();
    return mix(base, blend, alpha);
}

double luminance(const QColor& color) {
    double r = color.redF();
    double g = color.greenF();
    double b = color.blueF();
    
    // sRGB to linear
    auto toLinear = [](double c) {
        return c <= 0.03928 ? c / 12.92 : std::pow((c + 0.055) / 1.055, 2.4);
    };
    
    return 0.2126 * toLinear(r) + 0.7152 * toLinear(g) + 0.0722 * toLinear(b);
}

double contrast(const QColor& a, const QColor& b) {
    double l1 = luminance(a);
    double l2 = luminance(b);
    
    if (l1 > l2) {
        return (l1 + 0.05) / (l2 + 0.05);
    }
    return (l2 + 0.05) / (l1 + 0.05);
}

bool isDark(const QColor& color) {
    return luminance(color) < 0.5;
}

bool isLight(const QColor& color) {
    return !isDark(color);
}

QColor contrastingText(const QColor& background) {
    return isDark(background) ? Qt::white : Qt::black;
}

QColor random() {
    return QColor(
        QRandomGenerator::global()->bounded(256),
        QRandomGenerator::global()->bounded(256),
        QRandomGenerator::global()->bounded(256)
    );
}

QColor randomPastel() {
    int h = QRandomGenerator::global()->bounded(360);
    QColor c;
    c.setHsl(h, 128, 200);
    return c;
}

QColor randomVibrant() {
    int h = QRandomGenerator::global()->bounded(360);
    QColor c;
    c.setHsl(h, 255, 128);
    return c;
}

QList<QColor> palette(const QColor& base, int count) {
    QList<QColor> colors;
    int h, s, l;
    base.getHsl(&h, &s, &l);
    
    int step = 360 / count;
    for (int i = 0; i < count; i++) {
        QColor c;
        c.setHsl((h + i * step) % 360, s, l);
        colors.append(c);
    }
    
    return colors;
}

QList<QColor> gradient(const QColor& start, const QColor& end, int steps) {
    QList<QColor> colors;
    
    for (int i = 0; i < steps; i++) {
        double t = static_cast<double>(i) / (steps - 1);
        colors.append(mix(start, end, t));
    }
    
    return colors;
}

} // namespace Color

// ============================================================================
// ANIMATION ENGINE
// ============================================================================

AnimationEngine* AnimationEngine::s_instance = nullptr;

AnimationEngine* AnimationEngine::instance() {
    if (!s_instance) {
        s_instance = new AnimationEngine();
    }
    return s_instance;
}

AnimationEngine::AnimationEngine() : QObject(nullptr) {}

QPropertyAnimation* AnimationEngine::animate(
    QObject* target, const QByteArray& property,
    const QVariant& endValue, int duration, Easing easing)
{
    auto* anim = new QPropertyAnimation(target, property, this);
    anim->setEndValue(endValue);
    anim->setDuration(duration);
    anim->setEasingCurve(toQtEasing(easing));
    
    m_animations[target].append(anim);
    
    connect(anim, &QPropertyAnimation::finished, [this, target, anim]() {
        m_animations[target].removeAll(anim);
        anim->deleteLater();
    });
    
    emit animationStarted(target, QString::fromLatin1(property));
    anim->start();
    
    return anim;
}

QPropertyAnimation* AnimationEngine::animate(
    QObject* target, const QByteArray& property,
    const QVariant& startValue, const QVariant& endValue,
    int duration, Easing easing)
{
    auto* anim = new QPropertyAnimation(target, property, this);
    anim->setStartValue(startValue);
    anim->setEndValue(endValue);
    anim->setDuration(duration);
    anim->setEasingCurve(toQtEasing(easing));
    
    m_animations[target].append(anim);
    
    connect(anim, &QPropertyAnimation::finished, [this, target, anim]() {
        m_animations[target].removeAll(anim);
        anim->deleteLater();
    });
    
    emit animationStarted(target, QString::fromLatin1(property));
    anim->start();
    
    return anim;
}

QParallelAnimationGroup* AnimationEngine::parallel() {
    return new QParallelAnimationGroup(this);
}

QSequentialAnimationGroup* AnimationEngine::sequential() {
    return new QSequentialAnimationGroup(this);
}

void AnimationEngine::stopAll(QObject* target) {
    if (m_animations.contains(target)) {
        for (auto* anim : m_animations[target]) {
            anim->stop();
            anim->deleteLater();
        }
        m_animations[target].clear();
    }
}

void AnimationEngine::pauseAll(QObject* target) {
    if (m_animations.contains(target)) {
        for (auto* anim : m_animations[target]) {
            anim->pause();
        }
    }
}

void AnimationEngine::resumeAll(QObject* target) {
    if (m_animations.contains(target)) {
        for (auto* anim : m_animations[target]) {
            anim->resume();
        }
    }
}

QEasingCurve::Type AnimationEngine::toQtEasing(Easing easing) {
    switch (easing) {
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

void AnimationEngine::fadeIn(QWidget* widget, int duration) {
    widget->setWindowOpacity(0);
    widget->show();
    animate(widget, "windowOpacity", 0.0, 1.0, duration, Easing::OutCubic);
}

void AnimationEngine::fadeOut(QWidget* widget, int duration) {
    auto* anim = animate(widget, "windowOpacity", widget->windowOpacity(), 0.0, duration, Easing::OutCubic);
    connect(anim, &QPropertyAnimation::finished, widget, &QWidget::hide);
}

void AnimationEngine::slideIn(QWidget* widget, Position from, int duration) {
    QPoint target = widget->pos();
    QPoint start = target;
    QSize screen = Screen::size();
    
    switch (from) {
        case Position::TopCenter:
        case Position::TopLeft:
        case Position::TopRight:
            start.setY(-widget->height());
            break;
        case Position::BottomCenter:
        case Position::BottomLeft:
        case Position::BottomRight:
            start.setY(screen.height());
            break;
        case Position::CenterLeft:
            start.setX(-widget->width());
            break;
        case Position::CenterRight:
            start.setX(screen.width());
            break;
        default:
            break;
    }
    
    widget->move(start);
    widget->show();
    animate(widget, "pos", start, target, duration, Easing::OutCubic);
}

void AnimationEngine::slideOut(QWidget* widget, Position to, int duration) {
    QPoint start = widget->pos();
    QPoint target = start;
    QSize screen = Screen::size();
    
    switch (to) {
        case Position::TopCenter:
        case Position::TopLeft:
        case Position::TopRight:
            target.setY(-widget->height());
            break;
        case Position::BottomCenter:
        case Position::BottomLeft:
        case Position::BottomRight:
            target.setY(screen.height());
            break;
        case Position::CenterLeft:
            target.setX(-widget->width());
            break;
        case Position::CenterRight:
            target.setX(screen.width());
            break;
        default:
            break;
    }
    
    auto* anim = animate(widget, "pos", start, target, duration, Easing::InCubic);
    connect(anim, &QPropertyAnimation::finished, widget, &QWidget::hide);
}

void AnimationEngine::bounce(QWidget* widget, int duration) {
    auto* group = sequential();
    
    QRect orig = widget->geometry();
    QRect up = orig.adjusted(0, -10, 0, -10);
    
    auto* anim1 = new QPropertyAnimation(widget, "geometry");
    anim1->setDuration(duration / 4);
    anim1->setStartValue(orig);
    anim1->setEndValue(up);
    anim1->setEasingCurve(QEasingCurve::OutQuad);
    
    auto* anim2 = new QPropertyAnimation(widget, "geometry");
    anim2->setDuration(duration * 3 / 4);
    anim2->setStartValue(up);
    anim2->setEndValue(orig);
    anim2->setEasingCurve(QEasingCurve::OutBounce);
    
    group->addAnimation(anim1);
    group->addAnimation(anim2);
    group->start(QAbstractAnimation::DeleteWhenStopped);
}

void AnimationEngine::pulse(QWidget* widget, int duration, int count) {
    auto* anim = new QPropertyAnimation(widget, "windowOpacity", this);
    anim->setDuration(duration);
    anim->setStartValue(1.0);
    anim->setKeyValueAt(0.5, 0.5);
    anim->setEndValue(1.0);
    anim->setEasingCurve(QEasingCurve::InOutSine);
    anim->setLoopCount(count);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void AnimationEngine::shake(QWidget* widget, int duration, int intensity) {
    auto* anim = new QPropertyAnimation(widget, "pos", this);
    anim->setDuration(duration);
    
    QPoint orig = widget->pos();
    anim->setStartValue(orig);
    anim->setKeyValueAt(0.1, orig + QPoint(intensity, 0));
    anim->setKeyValueAt(0.2, orig + QPoint(-intensity, 0));
    anim->setKeyValueAt(0.3, orig + QPoint(intensity, 0));
    anim->setKeyValueAt(0.4, orig + QPoint(-intensity, 0));
    anim->setKeyValueAt(0.5, orig + QPoint(intensity, 0));
    anim->setKeyValueAt(0.6, orig + QPoint(-intensity, 0));
    anim->setKeyValueAt(0.7, orig + QPoint(intensity, 0));
    anim->setKeyValueAt(0.8, orig + QPoint(-intensity, 0));
    anim->setEndValue(orig);
    
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void AnimationEngine::scale(QWidget* widget, double from, double to, int duration) {
    QRect orig = widget->geometry();
    QRect start = orig;
    QRect end = orig;
    
    int fromW = static_cast<int>(orig.width() * from);
    int fromH = static_cast<int>(orig.height() * from);
    int toW = static_cast<int>(orig.width() * to);
    int toH = static_cast<int>(orig.height() * to);
    
    start.setSize(QSize(fromW, fromH));
    start.moveCenter(orig.center());
    end.setSize(QSize(toW, toH));
    end.moveCenter(orig.center());
    
    animate(widget, "geometry", start, end, duration, Easing::OutCubic);
}

AnimationEngine* anim() {
    return AnimationEngine::instance();
}

// ============================================================================
// LOGGER
// ============================================================================

Logger* Logger::s_instance = nullptr;

Logger* Logger::instance() {
    if (!s_instance) {
        s_instance = new Logger();
    }
    return s_instance;
}

Logger::Logger() : QObject(nullptr) {}

void Logger::debug(const QString& message) {
    log(Debug, "milk", message);
}

void Logger::info(const QString& message) {
    log(Info, "milk", message);
}

void Logger::warning(const QString& message) {
    log(Warning, "milk", message);
}

void Logger::error(const QString& message) {
    log(Error, "milk", message);
}

void Logger::fatal(const QString& message) {
    log(Fatal, "milk", message);
}

void Logger::log(Level level, const QString& category, const QString& message) {
    if (level < m_level) return;
    
    QString formatted = formatMessage(level, category, message);
    
    if (m_logToConsole) {
        QTextStream out(level >= Error ? stderr : stdout);
        out << formatted << "\n";
    }
    
    if (m_logToFile && !m_logPath.isEmpty()) {
        QFile file(m_logPath);
        if (file.open(QIODevice::Append | QIODevice::Text)) {
            QTextStream out(&file);
            out << formatted << "\n";
        }
    }
    
    emit logged(level, category, message);
}

void Logger::setLogLevel(Level level) {
    m_level = level;
}

void Logger::setLogToFile(bool enabled, const QString& path) {
    m_logToFile = enabled;
    if (!path.isEmpty()) {
        m_logPath = path;
    } else if (m_logPath.isEmpty()) {
        m_logPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/milkwidget.log";
    }
}

void Logger::setLogToConsole(bool enabled) {
    m_logToConsole = enabled;
}

void Logger::setColorOutput(bool enabled) {
    m_colorOutput = enabled;
}

void Logger::setFormat(const QString& format) {
    m_format = format;
}

QString Logger::formatMessage(Level level, const QString& category, const QString& message) {
    QString result = m_format;
    
    QString levelStr;
    QString colorCode;
    
    switch (level) {
        case Debug: levelStr = "DEBUG"; colorCode = "\033[36m"; break;
        case Info: levelStr = "INFO"; colorCode = "\033[32m"; break;
        case Warning: levelStr = "WARN"; colorCode = "\033[33m"; break;
        case Error: levelStr = "ERROR"; colorCode = "\033[31m"; break;
        case Fatal: levelStr = "FATAL"; colorCode = "\033[35m"; break;
    }
    
    result.replace("%time%", QTime::currentTime().toString("hh:mm:ss.zzz"));
    result.replace("%date%", QDate::currentDate().toString("yyyy-MM-dd"));
    result.replace("%level%", levelStr);
    result.replace("%category%", category);
    result.replace("%message%", message);
    
    if (m_colorOutput && m_logToConsole) {
        result = colorCode + result + "\033[0m";
    }
    
    return result;
}

Logger* log() {
    return Logger::instance();
}

// ============================================================================
// FILE UTILITIES
// ============================================================================

namespace File {

QString readText(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QString();
    }
    return QString::fromUtf8(file.readAll());
}

QByteArray readBytes(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return QByteArray();
    }
    return file.readAll();
}

bool writeText(const QString& path, const QString& content) {
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    QTextStream out(&file);
    out << content;
    return true;
}

bool writeBytes(const QString& path, const QByteArray& data) {
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    return file.write(data) == data.size();
}

bool exists(const QString& path) {
    return QFileInfo::exists(path);
}

bool isFile(const QString& path) {
    return QFileInfo(path).isFile();
}

bool isDirectory(const QString& path) {
    return QFileInfo(path).isDir();
}

QString baseName(const QString& path) {
    return QFileInfo(path).fileName();
}

QString dirName(const QString& path) {
    return QFileInfo(path).dir().path();
}

QString extension(const QString& path) {
    return QFileInfo(path).suffix();
}

QString join(const QString& a, const QString& b) {
    return QDir(a).filePath(b);
}

QString absolute(const QString& path) {
    return QFileInfo(path).absoluteFilePath();
}

QString relative(const QString& path, const QString& base) {
    return QDir(base).relativeFilePath(path);
}

bool mkdirs(const QString& path) {
    return QDir().mkpath(path);
}

QStringList listFiles(const QString& path, const QStringList& filters) {
    QDir dir(path);
    if (filters.isEmpty()) {
        return dir.entryList(QDir::Files);
    }
    return dir.entryList(filters, QDir::Files);
}

QStringList listDirs(const QString& path) {
    QDir dir(path);
    return dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
}

QString configDir() {
    return QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/milkwidget";
}

QString dataDir() {
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
}

QString cacheDir() {
    return QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
}

QString homeDir() {
    return QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
}

QString tempDir() {
    return QStandardPaths::writableLocation(QStandardPaths::TempLocation);
}

} // namespace File

// ============================================================================
// STRING UTILITIES
// ============================================================================

namespace String {

QString formatBytes(qint64 bytes) {
    const char* units[] = {"B", "KB", "MB", "GB", "TB", "PB"};
    int unitIndex = 0;
    double value = static_cast<double>(bytes);
    
    while (value >= 1024 && unitIndex < 5) {
        value /= 1024;
        unitIndex++;
    }
    
    if (unitIndex == 0) {
        return QString("%1 %2").arg(bytes).arg(units[0]);
    }
    return QString("%1 %2").arg(value, 0, 'f', 1).arg(units[unitIndex]);
}

QString formatDuration(int seconds) {
    int days = seconds / 86400;
    int hours = (seconds % 86400) / 3600;
    int minutes = (seconds % 3600) / 60;
    int secs = seconds % 60;
    
    if (days > 0) {
        return QString("%1d %2h %3m").arg(days).arg(hours).arg(minutes);
    }
    if (hours > 0) {
        return QString("%1h %2m %3s").arg(hours).arg(minutes).arg(secs);
    }
    if (minutes > 0) {
        return QString("%1m %2s").arg(minutes).arg(secs);
    }
    return QString("%1s").arg(secs);
}

QString formatDurationMs(int milliseconds) {
    return formatDuration(milliseconds / 1000);
}

QString formatPercent(double value, int decimals) {
    return QString("%1%").arg(value, 0, 'f', decimals);
}

QString formatTemperature(double celsius, bool fahrenheit) {
    if (fahrenheit) {
        double f = celsius * 9.0 / 5.0 + 32;
        return QString("%1°F").arg(f, 0, 'f', 1);
    }
    return QString("%1°C").arg(celsius, 0, 'f', 1);
}

QString toCamelCase(const QString& str) {
    QString result;
    bool capitalizeNext = false;
    
    for (const QChar& c : str) {
        if (c == '_' || c == '-' || c == ' ') {
            capitalizeNext = true;
        } else if (capitalizeNext) {
            result += c.toUpper();
            capitalizeNext = false;
        } else {
            result += c.toLower();
        }
    }
    
    return result;
}

QString toSnakeCase(const QString& str) {
    QString result;
    
    for (int i = 0; i < str.length(); i++) {
        QChar c = str[i];
        if (c.isUpper() && i > 0) {
            result += '_';
        }
        result += c.toLower();
    }
    
    return result;
}

QString toKebabCase(const QString& str) {
    return toSnakeCase(str).replace('_', '-');
}

QString toTitleCase(const QString& str) {
    QString result;
    bool capitalizeNext = true;
    
    for (const QChar& c : str) {
        if (c.isSpace()) {
            result += c;
            capitalizeNext = true;
        } else if (capitalizeNext) {
            result += c.toUpper();
            capitalizeNext = false;
        } else {
            result += c.toLower();
        }
    }
    
    return result;
}

QString truncate(const QString& str, int maxLength, const QString& suffix) {
    if (str.length() <= maxLength) return str;
    return str.left(maxLength - suffix.length()) + suffix;
}

QString ellipsis(const QString& str, int maxLength) {
    return truncate(str, maxLength, "...");
}

int toInt(const QString& str, int defaultValue) {
    bool ok;
    int value = str.toInt(&ok);
    return ok ? value : defaultValue;
}

double toDouble(const QString& str, double defaultValue) {
    bool ok;
    double value = str.toDouble(&ok);
    return ok ? value : defaultValue;
}

bool toBool(const QString& str, bool defaultValue) {
    QString s = str.toLower().trimmed();
    if (s == "true" || s == "yes" || s == "1" || s == "on") return true;
    if (s == "false" || s == "no" || s == "0" || s == "off") return false;
    return defaultValue;
}

} // namespace String

// ============================================================================
// TIMER UTILITIES
// ============================================================================

QTimer* createTimer(int interval, std::function<void()> callback) {
    QTimer* timer = new QTimer();
    QObject::connect(timer, &QTimer::timeout, callback);
    timer->start(interval);
    return timer;
}

void delay(int ms, std::function<void()> callback) {
    QTimer::singleShot(ms, callback);
}

void debounce(QObject* context, int ms, std::function<void()> callback) {
    static QMap<QObject*, QTimer*> timers;
    
    if (timers.contains(context)) {
        timers[context]->stop();
        timers[context]->deleteLater();
    }
    
    QTimer* timer = new QTimer(context);
    timer->setSingleShot(true);
    QObject::connect(timer, &QTimer::timeout, [callback, context, &timers]() {
        callback();
        timers.remove(context);
    });
    
    timers[context] = timer;
    timer->start(ms);
}

void throttle(QObject* context, int ms, std::function<void()> callback) {
    static QMap<QObject*, qint64> lastCalls;
    
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    
    if (!lastCalls.contains(context) || now - lastCalls[context] >= ms) {
        callback();
        lastCalls[context] = now;
    }
}

// ============================================================================
// SCREEN UTILITIES
// ============================================================================

namespace Screen {

QSize size() {
    QScreen* screen = QGuiApplication::primaryScreen();
    return screen ? screen->size() : QSize(1920, 1080);
}

QRect geometry() {
    QScreen* screen = QGuiApplication::primaryScreen();
    return screen ? screen->geometry() : QRect(0, 0, 1920, 1080);
}

QRect availableGeometry() {
    QScreen* screen = QGuiApplication::primaryScreen();
    return screen ? screen->availableGeometry() : QRect(0, 0, 1920, 1080);
}

QPoint center() {
    QSize s = size();
    return QPoint(s.width() / 2, s.height() / 2);
}

int screenAt(const QPoint& point) {
    QList<QScreen*> screens = QGuiApplication::screens();
    for (int i = 0; i < screens.size(); i++) {
        if (screens[i]->geometry().contains(point)) {
            return i;
        }
    }
    return 0;
}

int screenCount() {
    return QGuiApplication::screens().size();
}

double dpi() {
    QScreen* screen = QGuiApplication::primaryScreen();
    return screen ? screen->logicalDotsPerInch() : 96.0;
}

double scaleFactor() {
    QScreen* screen = QGuiApplication::primaryScreen();
    return screen ? screen->devicePixelRatio() : 1.0;
}

QPoint calculatePosition(Position pos, const QSize& widgetSize, int margin) {
    QRect avail = availableGeometry();
    int x = 0, y = 0;
    
    switch (pos) {
        case Position::TopLeft:
            x = avail.left() + margin;
            y = avail.top() + margin;
            break;
        case Position::TopCenter:
            x = avail.center().x() - widgetSize.width() / 2;
            y = avail.top() + margin;
            break;
        case Position::TopRight:
            x = avail.right() - widgetSize.width() - margin;
            y = avail.top() + margin;
            break;
        case Position::CenterLeft:
            x = avail.left() + margin;
            y = avail.center().y() - widgetSize.height() / 2;
            break;
        case Position::Center:
            x = avail.center().x() - widgetSize.width() / 2;
            y = avail.center().y() - widgetSize.height() / 2;
            break;
        case Position::CenterRight:
            x = avail.right() - widgetSize.width() - margin;
            y = avail.center().y() - widgetSize.height() / 2;
            break;
        case Position::BottomLeft:
            x = avail.left() + margin;
            y = avail.bottom() - widgetSize.height() - margin;
            break;
        case Position::BottomCenter:
            x = avail.center().x() - widgetSize.width() / 2;
            y = avail.bottom() - widgetSize.height() - margin;
            break;
        case Position::BottomRight:
            x = avail.right() - widgetSize.width() - margin;
            y = avail.bottom() - widgetSize.height() - margin;
            break;
        case Position::Manual:
            break;
    }
    
    return QPoint(x, y);
}

} // namespace Screen

} // namespace Milk
