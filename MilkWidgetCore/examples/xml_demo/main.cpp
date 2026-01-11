/**
 * MilkWidgetCore - XML Widget Demo
 * 
 * Demonstrates loading widgets from XML configuration
 */

#include <milk/MilkWidget.h>
#include <QFile>
#include <QTextStream>

using namespace Milk;

// Create a sample XML config if it doesn't exist
void createSampleConfig(const QString& path) {
    QFile file(path);
    if (file.exists()) return;
    
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return;
    
    QTextStream out(&file);
    out << R"(<?xml version="1.0" encoding="UTF-8"?>
<widgets>
    <!-- System Monitor Widget -->
    <widget width="320" height="200" 
            background="rgba(30,35,45,220)" 
            rounded="10" 
            position="top-right"
            glass="true">
        
        <title color="#4A9EFF">System Info</title>
        
        <label color="#888888" style="caption">CPU Usage</label>
        <progress id="cpu" bg="#333" color="#FF6B6B" height="10" rounded="5"/>
        
        <label color="#888888" style="caption">Memory</label>
        <progress id="mem" bg="#333" color="#4ECDC4" height="10" rounded="5"/>
        
        <spacer size="10"/>
        
        <label id="info" color="#AAAAAA" style="monospace"/>
    </widget>
    
    <!-- Clock Widget -->
    <widget width="180" height="80"
            background="rgba(40,30,60,200)"
            rounded="12"
            position="top-left">
        
        <clock style="digital" color="#FFFFFF" show-seconds="true" show-date="false"/>
    </widget>
    
    <!-- Decorative Circle -->
    <widget width="80" height="80"
            shape="circle"
            background="rgba(80,40,120,180)"
            position="bottom-center"
            glow="#9C27B0 12">
        
        <text align="center" color="#FFFFFF" bold="true">M</text>
    </widget>
</widgets>
)";
    file.close();
}

int main(int argc, char *argv[])
{
    Application app(argc, argv);
    
    // Create sample config
    QString configPath = app.configDir() + "/demo.xml";
    createSampleConfig(configPath);
    
    // Load widgets from XML
    QList<Widget*> widgets = app.loadWidgets(configPath);
    
    if (widgets.isEmpty()) {
        log()->error("Failed to load widgets from: " + configPath);
        return 1;
    }
    
    log()->info(QString("Loaded %1 widgets from XML").arg(widgets.size()));
    
    // Show all loaded widgets
    app.showAll();
    
    // Enable system tray
    app.enableTrayIcon(true);
    
    // Set up system monitor updates if we have a system monitor widget
    SystemMonitor* sys = SystemMonitor::instance();
    
    QTimer updateTimer;
    QObject::connect(&updateTimer, &QTimer::timeout, [&]() {
        // Update would happen here if we had widget IDs
        // For now, widgets auto-update via their own timers
    });
    updateTimer.start(1000);
    
    return app.exec();
}
