/**
 * MilkWidgetCore - XML Parser Implementation
 */

#include "milk/Parsers.h"
#include "milk/Widget.h"
#include "milk/Widgets.h"
#include "milk/Utils.h"

#include <QFile>
#include <QFileInfo>
#include <QTextStream>

namespace Milk {

XMLParser::XMLParser(QObject* parent)
    : QObject(parent)
{
}

QList<Widget*> XMLParser::parseFile(const QString& path) {
    m_lastError.clear();
    QList<Widget*> widgets;
    
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_lastError = QString("Cannot open file: %1").arg(path);
        emit parseError(m_lastError, 0, 0);
        return widgets;
    }
    
    m_basePath = QFileInfo(path).absolutePath();
    
    QDomDocument doc;
    QString errorMsg;
    int errorLine, errorColumn;
    
    if (!doc.setContent(&file, &errorMsg, &errorLine, &errorColumn)) {
        m_lastError = QString("XML parse error at line %1, column %2: %3")
            .arg(errorLine).arg(errorColumn).arg(errorMsg);
        emit parseError(m_lastError, errorLine, errorColumn);
        file.close();
        return widgets;
    }
    
    file.close();
    
    QDomElement root = doc.documentElement();
    
    // Handle root element
    if (root.tagName() == "widgets" || root.tagName() == "milk") {
        // Container with multiple widgets
        QDomNode child = root.firstChild();
        while (!child.isNull()) {
            if (child.isElement()) {
                QDomElement elem = child.toElement();
                if (elem.tagName() == "widget") {
                    Widget* w = parseWidget(elem);
                    if (w) {
                        widgets.append(w);
                        emit widgetCreated(w);
                    }
                }
            }
            child = child.nextSibling();
        }
    } else if (root.tagName() == "widget") {
        // Single widget
        Widget* w = parseWidget(root);
        if (w) {
            widgets.append(w);
            emit widgetCreated(w);
        }
    }
    
    return widgets;
}

QList<Widget*> XMLParser::parseString(const QString& xml) {
    m_lastError.clear();
    QList<Widget*> widgets;
    
    QDomDocument doc;
    QString errorMsg;
    int errorLine, errorColumn;
    
    if (!doc.setContent(xml, &errorMsg, &errorLine, &errorColumn)) {
        m_lastError = QString("XML parse error at line %1, column %2: %3")
            .arg(errorLine).arg(errorColumn).arg(errorMsg);
        emit parseError(m_lastError, errorLine, errorColumn);
        return widgets;
    }
    
    QDomElement root = doc.documentElement();
    
    if (root.tagName() == "widgets" || root.tagName() == "milk") {
        QDomNode child = root.firstChild();
        while (!child.isNull()) {
            if (child.isElement()) {
                QDomElement elem = child.toElement();
                if (elem.tagName() == "widget") {
                    Widget* w = parseWidget(elem);
                    if (w) widgets.append(w);
                }
            }
            child = child.nextSibling();
        }
    } else if (root.tagName() == "widget") {
        Widget* w = parseWidget(root);
        if (w) widgets.append(w);
    }
    
    return widgets;
}

Widget* XMLParser::parseWidget(const QDomElement& elem) {
    // Get dimensions
    int width = elem.attribute("width", "300").toInt();
    int height = elem.attribute("height", "200").toInt();
    
    Widget* widget = Widget::create(width, height);
    
    // Parse properties
    parseWidgetProperties(widget, elem);
    
    // Parse children
    parseChildren(widget, elem);
    
    return widget;
}

void XMLParser::parseWidgetProperties(Widget* widget, const QDomElement& elem) {
    // Background
    if (elem.hasAttribute("background")) {
        widget->setBackground(elem.attribute("background"));
    }
    if (elem.hasAttribute("bg")) {
        widget->setBackground(elem.attribute("bg"));
    }
    
    // Shape
    if (elem.hasAttribute("shape")) {
        widget->setShape(parseShape(elem.attribute("shape")));
    }
    if (elem.hasAttribute("rounded")) {
        widget->setRounded(elem.attribute("rounded").toInt());
    }
    if (elem.hasAttribute("radius")) {
        widget->setRounded(elem.attribute("radius").toInt());
    }
    
    // Position
    if (elem.hasAttribute("position")) {
        widget->setPosition(parsePosition(elem.attribute("position")));
    }
    if (elem.hasAttribute("pos")) {
        widget->setPosition(parsePosition(elem.attribute("pos")));
    }
    if (elem.hasAttribute("x") && elem.hasAttribute("y")) {
        widget->setPosition(
            elem.attribute("x").toInt(),
            elem.attribute("y").toInt()
        );
    }
    
    // Border
    if (elem.hasAttribute("border")) {
        QString border = elem.attribute("border");
        QStringList parts = border.split(' ');
        if (parts.size() >= 2) {
            widget->setBorder(parts[1], parts[0].remove("px").toInt());
        } else {
            widget->setBorder(border, 1);
        }
    }
    if (elem.hasAttribute("border-color") && elem.hasAttribute("border-width")) {
        widget->setBorder(
            elem.attribute("border-color"),
            elem.attribute("border-width").toInt()
        );
    }
    
    // Effects
    if (elem.hasAttribute("opacity")) {
        widget->setOpacity(elem.attribute("opacity").toDouble());
    }
    if (elem.hasAttribute("glass")) {
        widget->setGlass(elem.attribute("glass") == "true");
    }
    if (elem.hasAttribute("blur")) {
        widget->setBlur(BlurMode::Glass, elem.attribute("blur").toDouble());
    }
    if (elem.hasAttribute("glow")) {
        QString glow = elem.attribute("glow");
        QStringList parts = glow.split(' ');
        if (parts.size() >= 2) {
            widget->setGlow(parts[0], parts[1].toInt());
        } else {
            widget->setGlow(glow);
        }
    }
    if (elem.hasAttribute("shadow")) {
        QString shadow = elem.attribute("shadow");
        // Parse shadow: "color blur offsetX offsetY"
        QStringList parts = shadow.split(' ');
        if (parts.size() >= 4) {
            widget->setShadow(
                parseColor(parts[0]),
                parts[1].toInt(),
                parts[2].toInt(),
                parts[3].toInt()
            );
        }
    }
    
    // Behavior
    if (elem.hasAttribute("draggable")) {
        widget->setDraggable(elem.attribute("draggable") == "true");
    }
    if (elem.hasAttribute("always-on-top")) {
        widget->setAlwaysOnTop(elem.attribute("always-on-top") == "true");
    }
    if (elem.hasAttribute("click-through")) {
        widget->setClickThrough(elem.attribute("click-through") == "true");
    }
    
    // Layout
    if (elem.hasAttribute("margin")) {
        widget->setMargin(elem.attribute("margin").toInt());
    }
    if (elem.hasAttribute("padding")) {
        widget->setPadding(elem.attribute("padding").toInt());
    }
    if (elem.hasAttribute("spacing")) {
        widget->setSpacing(elem.attribute("spacing").toInt());
    }
    
    // Style class
    if (elem.hasAttribute("class")) {
        widget->setStyleClass(elem.attribute("class"));
    }
}

void XMLParser::parseChildren(Widget* parent, const QDomElement& elem) {
    QDomNode child = elem.firstChild();
    while (!child.isNull()) {
        if (child.isElement()) {
            QDomElement childElem = child.toElement();
            QWidget* childWidget = parseChildElement(childElem, parent);
            if (childWidget) {
                parent->addWidget(childWidget);
            }
        }
        child = child.nextSibling();
    }
}

QWidget* XMLParser::parseChildElement(const QDomElement& elem, Widget* parent) {
    QString tag = elem.tagName().toLower();
    
    // Text elements
    if (tag == "text" || tag == "label") {
        Text* text = Text::create(elem.text(), parent);
        
        if (elem.hasAttribute("color")) {
            text->setColor(elem.attribute("color"));
        }
        if (elem.hasAttribute("font")) {
            QString font = elem.attribute("font");
            QStringList parts = font.split(' ');
            if (parts.size() >= 2) {
                text->setFont(parts[0], parts[1].remove("px").toInt());
            } else {
                text->setFont(font);
            }
        }
        if (elem.hasAttribute("size")) {
            text->setFontSize(elem.attribute("size").toInt());
        }
        if (elem.hasAttribute("bold")) {
            text->setBold(elem.attribute("bold") == "true");
        }
        if (elem.hasAttribute("italic")) {
            text->setItalic(elem.attribute("italic") == "true");
        }
        if (elem.hasAttribute("align")) {
            text->setAlign(elem.attribute("align"));
        }
        if (elem.hasAttribute("glow")) {
            QString glow = elem.attribute("glow");
            QStringList parts = glow.split(' ');
            if (parts.size() >= 2) {
                text->setGlow(parts[0], parts[1].toInt());
            } else {
                text->setGlow(glow);
            }
        }
        if (elem.hasAttribute("style")) {
            QString style = elem.attribute("style");
            if (style == "title") text->setTitle();
            else if (style == "subtitle") text->setSubtitle();
            else if (style == "body") text->setBody();
            else if (style == "caption") text->setCaption();
            else if (style == "monospace" || style == "mono") text->setMonospace();
            else if (style == "code") text->setCode();
        }
        
        return text;
    }
    
    // Title (shorthand)
    if (tag == "title") {
        Text* text = Text::create(elem.text(), parent);
        text->setTitle();
        if (elem.hasAttribute("color")) {
            text->setColor(elem.attribute("color"));
        }
        return text;
    }
    
    // Progress bar
    if (tag == "progress" || tag == "progressbar" || tag == "progress-bar") {
        ProgressBar* bar = ProgressBar::create(parent);
        
        if (elem.hasAttribute("value")) {
            bar->setValue(elem.attribute("value").toDouble());
        }
        if (elem.hasAttribute("max")) {
            bar->setMaxValue(elem.attribute("max").toDouble());
        }
        if (elem.hasAttribute("min")) {
            bar->setMinValue(elem.attribute("min").toDouble());
        }
        if (elem.hasAttribute("background") && elem.hasAttribute("fill")) {
            bar->setColors(elem.attribute("background"), elem.attribute("fill"));
        }
        if (elem.hasAttribute("bg") && elem.hasAttribute("color")) {
            bar->setColors(elem.attribute("bg"), elem.attribute("color"));
        }
        if (elem.hasAttribute("rounded")) {
            bar->setRounded(elem.attribute("rounded").toInt());
        }
        if (elem.hasAttribute("height")) {
            bar->setHeight(elem.attribute("height").toInt());
        }
        if (elem.hasAttribute("show-text")) {
            bar->setShowText(elem.attribute("show-text") == "true");
        }
        
        return bar;
    }
    
    // Graph
    if (tag == "graph" || tag == "chart") {
        Graph* graph = Graph::create(parent);
        
        if (elem.hasAttribute("type")) {
            QString type = elem.attribute("type").toLower();
            if (type == "line") graph->setGraphType(GraphType::Line);
            else if (type == "area") graph->setGraphType(GraphType::Area);
            else if (type == "bar") graph->setGraphType(GraphType::Bar);
            else if (type == "sparkline") graph->setGraphType(GraphType::Sparkline);
        }
        if (elem.hasAttribute("color")) {
            graph->setLineColor(parseColor(elem.attribute("color")));
        }
        if (elem.hasAttribute("fill")) {
            graph->setFillColor(parseColor(elem.attribute("fill")));
        }
        if (elem.hasAttribute("max-points")) {
            graph->setMaxPoints(elem.attribute("max-points").toInt());
        }
        if (elem.hasAttribute("min")) {
            graph->setMinValue(elem.attribute("min").toDouble());
        }
        if (elem.hasAttribute("max")) {
            graph->setMaxValue(elem.attribute("max").toDouble());
        }
        if (elem.hasAttribute("grid")) {
            graph->setShowGrid(elem.attribute("grid") == "true");
        }
        
        return graph;
    }
    
    // Gauge
    if (tag == "gauge" || tag == "meter") {
        Gauge* gauge = Gauge::create(parent);
        
        if (elem.hasAttribute("value")) {
            gauge->setValue(elem.attribute("value").toDouble());
        }
        if (elem.hasAttribute("min") && elem.hasAttribute("max")) {
            gauge->setRange(
                elem.attribute("min").toDouble(),
                elem.attribute("max").toDouble()
            );
        }
        if (elem.hasAttribute("style")) {
            QString style = elem.attribute("style").toLower();
            if (style == "arc") gauge->setStyle(GaugeStyle::Arc);
            else if (style == "circle") gauge->setStyle(GaugeStyle::Circle);
            else if (style == "linear") gauge->setStyle(GaugeStyle::Linear);
            else if (style == "semicircle") gauge->setStyle(GaugeStyle::Semicircle);
        }
        if (elem.hasAttribute("thickness")) {
            gauge->setThickness(elem.attribute("thickness").toInt());
        }
        if (elem.hasAttribute("label")) {
            gauge->setLabel(elem.attribute("label"));
        }
        if (elem.hasAttribute("unit")) {
            gauge->setUnit(elem.attribute("unit"));
        }
        
        return gauge;
    }
    
    // Image
    if (tag == "image" || tag == "img") {
        QString src = elem.attribute("src", elem.attribute("source"));
        Image* image = Image::create(src, parent);
        
        if (elem.hasAttribute("rounded")) {
            image->setRounded(elem.attribute("rounded").toInt());
        }
        if (elem.hasAttribute("circular")) {
            image->setCircular(elem.attribute("circular") == "true");
        }
        if (elem.hasAttribute("opacity")) {
            image->setOpacity(elem.attribute("opacity").toDouble());
        }
        
        return image;
    }
    
    // Button
    if (tag == "button") {
        Button* button = Button::create(elem.text(), parent);
        
        if (elem.hasAttribute("background")) {
            button->setBackground(parseColor(elem.attribute("background")));
        }
        if (elem.hasAttribute("color")) {
            button->setTextColor(parseColor(elem.attribute("color")));
        }
        if (elem.hasAttribute("rounded")) {
            button->setRounded(elem.attribute("rounded").toInt());
        }
        
        return button;
    }
    
    // Spacer
    if (tag == "spacer" || tag == "space") {
        int size = elem.attribute("size", "10").toInt();
        return Spacer::create(size, parent);
    }
    
    // Clock
    if (tag == "clock") {
        Clock::Style style = Clock::Digital;
        if (elem.hasAttribute("style")) {
            QString s = elem.attribute("style").toLower();
            if (s == "analog") style = Clock::Analog;
            else if (s == "minimal") style = Clock::Minimal;
        }
        Clock* clock = Clock::create(style, parent);
        
        if (elem.hasAttribute("format")) {
            clock->setFormat(elem.attribute("format"));
        }
        if (elem.hasAttribute("color")) {
            clock->setTextColor(parseColor(elem.attribute("color")));
        }
        if (elem.hasAttribute("show-seconds")) {
            clock->setShowSeconds(elem.attribute("show-seconds") == "true");
        }
        if (elem.hasAttribute("show-date")) {
            clock->setShowDate(elem.attribute("show-date") == "true");
        }
        if (elem.hasAttribute("24hour")) {
            clock->set24Hour(elem.attribute("24hour") == "true");
        }
        
        return clock;
    }
    
    // Calendar
    if (tag == "calendar") {
        return Calendar::create(parent);
    }
    
    // Container
    if (tag == "container" || tag == "box" || tag == "vbox" || tag == "hbox") {
        Container::Layout layout = Container::Vertical;
        if (tag == "hbox" || elem.attribute("layout") == "horizontal") {
            layout = Container::Horizontal;
        } else if (elem.attribute("layout") == "grid") {
            layout = Container::Grid;
        }
        
        Container* container = Container::create(layout, parent);
        
        if (elem.hasAttribute("spacing")) {
            container->setSpacing(elem.attribute("spacing").toInt());
        }
        if (elem.hasAttribute("margin")) {
            container->setMargins(elem.attribute("margin").toInt());
        }
        
        // Parse children recursively
        QDomNode child = elem.firstChild();
        while (!child.isNull()) {
            if (child.isElement()) {
                QWidget* childWidget = parseChildElement(child.toElement(), parent);
                if (childWidget) {
                    container->addWidget(childWidget);
                }
            }
            child = child.nextSibling();
        }
        
        return container;
    }
    
    return nullptr;
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

QColor XMLParser::parseColor(const QString& value) {
    return Color::parse(value);
}

Position XMLParser::parsePosition(const QString& value) {
    QString v = value.toLower().replace('-', ' ').replace('_', ' ');
    
    if (v == "top left" || v == "topleft") return Position::TopLeft;
    if (v == "top center" || v == "topcenter" || v == "top") return Position::TopCenter;
    if (v == "top right" || v == "topright") return Position::TopRight;
    if (v == "center left" || v == "centerleft" || v == "left") return Position::CenterLeft;
    if (v == "center" || v == "middle") return Position::Center;
    if (v == "center right" || v == "centerright" || v == "right") return Position::CenterRight;
    if (v == "bottom left" || v == "bottomleft") return Position::BottomLeft;
    if (v == "bottom center" || v == "bottomcenter" || v == "bottom") return Position::BottomCenter;
    if (v == "bottom right" || v == "bottomright") return Position::BottomRight;
    
    return Position::Center;
}

Shape XMLParser::parseShape(const QString& value) {
    QString v = value.toLower();
    
    if (v == "rectangle" || v == "rect") return Shape::Rectangle;
    if (v == "rounded" || v == "roundedrect") return Shape::RoundedRect;
    if (v == "circle") return Shape::Circle;
    if (v == "ellipse" || v == "oval") return Shape::Ellipse;
    if (v == "square") return Shape::Square;
    
    return Shape::Rectangle;
}

Alignment XMLParser::parseAlignment(const QString& value) {
    QString v = value.toLower();
    
    if (v == "left") return Alignment::Left;
    if (v == "center") return Alignment::Center;
    if (v == "right") return Alignment::Right;
    if (v == "top") return Alignment::Top;
    if (v == "bottom") return Alignment::Bottom;
    
    return Alignment::Left;
}

QString XMLParser::toXml(Widget* widget) {
    return widget ? widget->toXml() : QString();
}

bool XMLParser::saveToFile(Widget* widget, const QString& path) {
    if (!widget) return false;
    
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_lastError = QString("Cannot write to file: %1").arg(path);
        return false;
    }
    
    QTextStream out(&file);
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    out << widget->toXml();
    
    file.close();
    return true;
}

// ============================================================================
// GLOBAL FUNCTIONS
// ============================================================================

QList<Widget*> loadXml(const QString& path) {
    XMLParser parser;
    return parser.parseFile(path);
}

static XMLParser* s_globalXmlParser = nullptr;

XMLParser* globalXML() {
    if (!s_globalXmlParser) {
        s_globalXmlParser = new XMLParser();
    }
    return s_globalXmlParser;
}

} // namespace Milk
