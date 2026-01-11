/**
 * MilkWidgetCore - CSS Parser Implementation
 */

#include "milk/Parsers.h"
#include "milk/Widget.h"
#include "milk/Utils.h"

#include <QFile>
#include <QTextStream>
#include <QRegularExpression>

namespace Milk {

CSSParser::CSSParser(QObject* parent)
    : QObject(parent)
{
}

bool CSSParser::parseFile(const QString& path) {
    m_lastError.clear();
    
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_lastError = QString("Cannot open file: %1").arg(path);
        return false;
    }
    
    QString css = QString::fromUtf8(file.readAll());
    file.close();
    
    return parseString(css);
}

bool CSSParser::parseString(const QString& css) {
    m_lastError.clear();
    
    // Remove comments
    QString cleaned = css;
    QRegularExpression commentRe("/\\*.*?\\*/", QRegularExpression::DotMatchesEverythingOption);
    cleaned.remove(commentRe);
    
    // Parse rules
    QRegularExpression ruleRe("([^{}]+)\\s*\\{([^{}]*)\\}");
    QRegularExpressionMatchIterator it = ruleRe.globalMatch(cleaned);
    
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString selector = match.captured(1).trimmed();
        QString body = match.captured(2).trimmed();
        
        parseRule(selector, body);
    }
    
    return true;
}

void CSSParser::parseRule(const QString& selector, const QString& body) {
    StyleSheet style;
    
    // Parse properties
    QStringList properties = body.split(';', Qt::SkipEmptyParts);
    for (const QString& prop : properties) {
        int colonIdx = prop.indexOf(':');
        if (colonIdx < 0) continue;
        
        QString property = prop.left(colonIdx).trimmed().toLower();
        QString value = prop.mid(colonIdx + 1).trimmed();
        
        parseProperty(style, property, value);
    }
    
    // Handle multiple selectors
    QStringList selectors = selector.split(',');
    for (QString sel : selectors) {
        sel = sel.trimmed();
        m_styles[sel] = style;
    }
}

void CSSParser::parseProperty(StyleSheet& style, const QString& property, const QString& value) {
    // Background
    if (property == "background" || property == "background-color" || property == "bg") {
        if (value.startsWith("linear-gradient")) {
            style.backgroundGradient = parseGradient(value);
        } else {
            style.backgroundColor = parseColor(value);
        }
    }
    else if (property == "background-image") {
        if (value.startsWith("url(")) {
            style.backgroundImage = value.mid(4, value.length() - 5).remove('"').remove('\'');
        }
    }
    
    // Text
    else if (property == "color") {
        style.textColor = parseColor(value);
    }
    else if (property == "font-family") {
        style.fontFamily = value.remove('"').remove('\'');
    }
    else if (property == "font-size") {
        style.fontSize = parsePixels(value);
    }
    else if (property == "font-weight") {
        style.fontBold = (value == "bold" || value.toInt() >= 700);
    }
    else if (property == "font-style") {
        style.fontItalic = (value == "italic");
    }
    
    // Border
    else if (property == "border") {
        style.border = parseBorder(value);
    }
    else if (property == "border-color") {
        style.border.color = parseColor(value);
    }
    else if (property == "border-width") {
        style.border.width = parsePixels(value);
    }
    else if (property == "border-radius") {
        style.cornerRadius = parsePixels(value);
    }
    else if (property == "border-style") {
        if (value == "solid") style.border.style = BorderStyle::Solid;
        else if (value == "dashed") style.border.style = BorderStyle::Dashed;
        else if (value == "dotted") style.border.style = BorderStyle::Dotted;
        else if (value == "none") style.border.style = BorderStyle::None;
    }
    
    // Shadow
    else if (property == "box-shadow") {
        style.shadow = parseShadow(value);
    }
    
    // Margin & Padding
    else if (property == "margin") {
        style.margin = parseMargin(value);
    }
    else if (property == "margin-top") {
        style.margin.top = parsePixels(value);
    }
    else if (property == "margin-right") {
        style.margin.right = parsePixels(value);
    }
    else if (property == "margin-bottom") {
        style.margin.bottom = parsePixels(value);
    }
    else if (property == "margin-left") {
        style.margin.left = parsePixels(value);
    }
    else if (property == "padding") {
        style.padding = Padding(parseMargin(value).top, parseMargin(value).right,
                                parseMargin(value).bottom, parseMargin(value).left);
    }
    else if (property == "padding-top") {
        style.padding.top = parsePixels(value);
    }
    else if (property == "padding-right") {
        style.padding.right = parsePixels(value);
    }
    else if (property == "padding-bottom") {
        style.padding.bottom = parsePixels(value);
    }
    else if (property == "padding-left") {
        style.padding.left = parsePixels(value);
    }
    
    // Effects
    else if (property == "opacity") {
        style.opacity = value.toDouble();
    }
    else if (property == "backdrop-filter" || property == "blur") {
        if (value.contains("blur")) {
            style.blur = BlurMode::Background;
            QRegularExpression blurRe("blur\\((\\d+)");
            QRegularExpressionMatch match = blurRe.match(value);
            if (match.hasMatch()) {
                style.blurRadius = match.captured(1).toDouble();
            }
        }
    }
}

// ============================================================================
// VALUE PARSERS
// ============================================================================

QColor CSSParser::parseColor(const QString& value) {
    return Color::parse(value);
}

int CSSParser::parsePixels(const QString& value) {
    QString v = value.trimmed().toLower();
    v.remove("px");
    v.remove("pt");
    v.remove("em");
    v.remove("rem");
    return v.toInt();
}

Margin CSSParser::parseMargin(const QString& value) {
    QStringList parts = value.split(' ', Qt::SkipEmptyParts);
    Margin m;
    
    if (parts.size() == 1) {
        int v = parsePixels(parts[0]);
        m = Margin(v, v, v, v);
    } else if (parts.size() == 2) {
        int v = parsePixels(parts[0]);
        int h = parsePixels(parts[1]);
        m = Margin(v, h, v, h);
    } else if (parts.size() == 3) {
        m.top = parsePixels(parts[0]);
        m.right = parsePixels(parts[1]);
        m.left = parsePixels(parts[1]);
        m.bottom = parsePixels(parts[2]);
    } else if (parts.size() >= 4) {
        m.top = parsePixels(parts[0]);
        m.right = parsePixels(parts[1]);
        m.bottom = parsePixels(parts[2]);
        m.left = parsePixels(parts[3]);
    }
    
    return m;
}

Border CSSParser::parseBorder(const QString& value) {
    Border b;
    QStringList parts = value.split(' ', Qt::SkipEmptyParts);
    
    for (const QString& part : parts) {
        QString p = part.trimmed();
        
        // Check if it's a width
        if (p.endsWith("px") || p[0].isDigit()) {
            b.width = parsePixels(p);
        }
        // Check if it's a style
        else if (p == "solid" || p == "dashed" || p == "dotted" || p == "none") {
            if (p == "solid") b.style = BorderStyle::Solid;
            else if (p == "dashed") b.style = BorderStyle::Dashed;
            else if (p == "dotted") b.style = BorderStyle::Dotted;
            else b.style = BorderStyle::None;
        }
        // Otherwise it's a color
        else {
            b.color = parseColor(p);
        }
    }
    
    return b;
}

Shadow CSSParser::parseShadow(const QString& value) {
    Shadow s;
    s.enabled = true;
    
    // Parse: offsetX offsetY blur spread color
    // or: offsetX offsetY blur color
    QStringList parts = value.split(' ', Qt::SkipEmptyParts);
    
    int idx = 0;
    QList<int> numbers;
    QString colorStr;
    
    for (const QString& part : parts) {
        QString p = part.trimmed();
        
        // Check if it's a number
        QString numPart = p;
        numPart.remove("px");
        bool ok;
        int num = numPart.toInt(&ok);
        
        if (ok) {
            numbers.append(num);
        } else {
            // It's a color (could be at start or end)
            colorStr = p;
        }
    }
    
    if (numbers.size() >= 2) {
        s.offsetX = numbers[0];
        s.offsetY = numbers[1];
    }
    if (numbers.size() >= 3) {
        s.blur = numbers[2];
    }
    if (numbers.size() >= 4) {
        s.spread = numbers[3];
    }
    
    if (!colorStr.isEmpty()) {
        s.color = parseColor(colorStr);
    }
    
    return s;
}

Gradient CSSParser::parseGradient(const QString& value) {
    Gradient g;
    
    // linear-gradient(angle, color1, color2)
    QRegularExpression gradRe("linear-gradient\\s*\\(\\s*(\\d+)deg\\s*,\\s*([^,]+)\\s*,\\s*([^)]+)\\s*\\)");
    QRegularExpressionMatch match = gradRe.match(value);
    
    if (match.hasMatch()) {
        g.type = Gradient::Linear;
        g.angle = match.captured(1).toDouble();
        g.start = parseColor(match.captured(2).trimmed());
        g.end = parseColor(match.captured(3).trimmed());
    } else {
        // Try simpler format: linear-gradient(color1, color2)
        QRegularExpression simpleRe("linear-gradient\\s*\\(\\s*([^,]+)\\s*,\\s*([^)]+)\\s*\\)");
        match = simpleRe.match(value);
        if (match.hasMatch()) {
            g.type = Gradient::Linear;
            g.angle = 180;  // Default top to bottom
            g.start = parseColor(match.captured(1).trimmed());
            g.end = parseColor(match.captured(2).trimmed());
        }
    }
    
    return g;
}

// ============================================================================
// GETTERS
// ============================================================================

StyleSheet CSSParser::getStyle(const QString& className) {
    QString selector = className.startsWith('.') ? className : ("." + className);
    return m_styles.value(selector, StyleSheet());
}

StyleSheet CSSParser::getTypeStyle(const QString& typeName) {
    return m_styles.value(typeName.toLower(), StyleSheet());
}

void CSSParser::applyStyle(Widget* widget, const QString& className) {
    StyleSheet style = getStyle(className);
    widget->setStyle(style);
}

QStringList CSSParser::classNames() const {
    return m_styles.keys();
}

QString CSSParser::toCSS(const StyleSheet& style) {
    QString css;
    
    if (style.backgroundColor.isValid()) {
        css += QString("  background-color: %1;\n").arg(Color::toString(style.backgroundColor));
    }
    if (style.textColor.isValid()) {
        css += QString("  color: %1;\n").arg(Color::toString(style.textColor));
    }
    if (!style.fontFamily.isEmpty()) {
        css += QString("  font-family: \"%1\";\n").arg(style.fontFamily);
    }
    if (style.fontSize > 0) {
        css += QString("  font-size: %1px;\n").arg(style.fontSize);
    }
    if (style.fontBold) {
        css += "  font-weight: bold;\n";
    }
    if (style.fontItalic) {
        css += "  font-style: italic;\n";
    }
    if (style.border.width > 0) {
        css += QString("  border: %1px solid %2;\n")
            .arg(style.border.width)
            .arg(Color::toString(style.border.color));
    }
    if (style.cornerRadius > 0) {
        css += QString("  border-radius: %1px;\n").arg(style.cornerRadius);
    }
    if (style.shadow.enabled) {
        css += QString("  box-shadow: %1px %2px %3px %4;\n")
            .arg(style.shadow.offsetX)
            .arg(style.shadow.offsetY)
            .arg(style.shadow.blur)
            .arg(Color::toString(style.shadow.color));
    }
    if (style.opacity < 1.0) {
        css += QString("  opacity: %1;\n").arg(style.opacity);
    }
    
    return css;
}

StyleSheet CSSParser::merge(const StyleSheet& a, const StyleSheet& b) {
    StyleSheet result = a;
    
    // Override with values from b if they're set
    if (b.backgroundColor.isValid()) result.backgroundColor = b.backgroundColor;
    if (b.backgroundGradient.isValid()) result.backgroundGradient = b.backgroundGradient;
    if (!b.backgroundImage.isEmpty()) result.backgroundImage = b.backgroundImage;
    if (b.textColor.isValid()) result.textColor = b.textColor;
    if (!b.fontFamily.isEmpty()) result.fontFamily = b.fontFamily;
    if (b.fontSize > 0) result.fontSize = b.fontSize;
    if (b.fontBold) result.fontBold = b.fontBold;
    if (b.fontItalic) result.fontItalic = b.fontItalic;
    if (b.border.width > 0) result.border = b.border;
    if (b.shadow.enabled) result.shadow = b.shadow;
    if (b.margin.top > 0 || b.margin.right > 0 || b.margin.bottom > 0 || b.margin.left > 0) {
        result.margin = b.margin;
    }
    if (b.padding.top > 0 || b.padding.right > 0 || b.padding.bottom > 0 || b.padding.left > 0) {
        result.padding = b.padding;
    }
    if (b.cornerRadius > 0) result.cornerRadius = b.cornerRadius;
    if (b.opacity < 1.0) result.opacity = b.opacity;
    if (b.blur != BlurMode::None) {
        result.blur = b.blur;
        result.blurRadius = b.blurRadius;
    }
    
    return result;
}

// ============================================================================
// GLOBAL FUNCTIONS
// ============================================================================

static CSSParser* s_globalCssParser = nullptr;

bool loadCSS(const QString& path) {
    if (!s_globalCssParser) {
        s_globalCssParser = new CSSParser();
    }
    return s_globalCssParser->parseFile(path);
}

CSSParser* globalCSS() {
    if (!s_globalCssParser) {
        s_globalCssParser = new CSSParser();
    }
    return s_globalCssParser;
}

} // namespace Milk
