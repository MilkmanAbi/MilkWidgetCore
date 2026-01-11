/**
 * MilkWidgetCore - Example System Monitor
 * 
 * A Conky-style system monitor widget demonstrating the API
 */

#include <milk/MilkWidget.h>

using namespace Milk;

int main(int argc, char *argv[])
{
    Application app(argc, argv);
    
    // Enable system tray
    app.enableTrayIcon(true);
    app.setTrayTooltip("MilkWidget System Monitor");
    
    // ========================================================================
    // System Monitor Widget
    // ========================================================================
    Widget* monitor = Widget::create(350, 280);
    monitor->setBackground(20, 25, 35, 230);
    monitor->setRounded(12);
    monitor->setPosition(Position::TopRight);
    monitor->setGlass(true);
    monitor->setDraggable(true);
    
    // Title
    Text* title = text("System Monitor", monitor);
    title->setTitle();
    title->setColor("#4A9EFF");
    
    // Get system monitor
    SystemMonitor* sys = SystemMonitor::instance();
    
    // CPU section
    Text* cpuLabel = label("CPU Usage", monitor);
    cpuLabel->setColor("#CCCCCC");
    cpuLabel->setCaption();
    
    ProgressBar* cpuBar = progressBar(monitor);
    cpuBar->setColors("#333333", "#FF6B6B");
    cpuBar->setRounded(4);
    cpuBar->setHeight(12);
    
    // Memory section
    Text* memLabel = label("Memory Usage", monitor);
    memLabel->setColor("#CCCCCC");
    memLabel->setCaption();
    
    ProgressBar* memBar = progressBar(monitor);
    memBar->setColors("#333333", "#4ECDC4");
    memBar->setRounded(4);
    memBar->setHeight(12);
    
    // Disk section
    Text* diskLabel = label("Disk Usage", monitor);
    diskLabel->setColor("#CCCCCC");
    diskLabel->setCaption();
    
    ProgressBar* diskBar = progressBar(monitor);
    diskBar->setColors("#333333", "#FFE66D");
    diskBar->setRounded(4);
    diskBar->setHeight(12);
    
    // Info text
    Text* info = label("", monitor);
    info->setMonospace();
    info->setColor("#A8A8A8");
    info->setFontSize(10);
    
    // Update callback
    monitor->onUpdate([=]() {
        cpuBar->setValue(sys->cpu());
        memBar->setValue(sys->memory());
        diskBar->setValue(sys->disk("/"));
        
        QString infoText = QString("Uptime: %1\nProcesses: %2\nTemp: %3")
            .arg(sys->uptime())
            .arg(sys->processes())
            .arg(String::formatTemperature(sys->temperature()));
        info->setText(infoText);
    });
    monitor->setUpdateInterval(1000);
    
    // ========================================================================
    // Clock Widget
    // ========================================================================
    Widget* clockWidget = Widget::create(200, 100);
    clockWidget->setBackground(35, 30, 50, 200);
    clockWidget->setRounded(15);
    clockWidget->setPosition(Position::TopLeft);
    clockWidget->setGlass(true);
    
    Clock* clk = clock(Clock::Digital, clockWidget);
    clk->setTextColor(Qt::white);
    clk->set24Hour(true);
    clk->setShowDate(true);
    
    // ========================================================================
    // CPU Graph Widget
    // ========================================================================
    Widget* graphWidget = Widget::create(300, 150);
    graphWidget->setBackground(25, 30, 40, 220);
    graphWidget->setRounded(10);
    graphWidget->setPosition(Position::BottomRight);
    
    Text* graphTitle = label("CPU History", graphWidget);
    graphTitle->setColor("#7B68EE");
    graphTitle->setBold(true);
    
    Graph* cpuGraph = graph(graphWidget);
    cpuGraph->setGraphType(GraphType::Area);
    cpuGraph->setLineColor(QColor(123, 104, 238));
    cpuGraph->setFillColor(QColor(123, 104, 238, 80));
    cpuGraph->setMaxPoints(60);
    cpuGraph->setShowGrid(true);
    cpuGraph->setGridColor(QColor(255, 255, 255, 20));
    
    graphWidget->onUpdate([=]() {
        cpuGraph->addValue(sys->cpu());
    });
    graphWidget->setUpdateInterval(1000);
    
    // ========================================================================
    // Demo Circle Widget
    // ========================================================================
    Widget* demo = Widget::createCircle(100);
    demo->setBackground(60, 20, 80, 180);
    demo->setPosition(Position::BottomLeft);
    demo->setGlow("#FF4081", 15);
    
    Text* demoText = text("Milk", demo);
    demoText->setAlign("center");
    demoText->setColor("#FFFFFF");
    demoText->setBold(true);
    
    // Animate periodically
    QTimer* animTimer = new QTimer(&app);
    QObject::connect(animTimer, &QTimer::timeout, [=]() {
        static int counter = 0;
        switch (counter % 3) {
            case 0: demo->bounce(800); break;
            case 1: demo->pulse(1000); break;
            case 2: demo->shake(500, 5); break;
        }
        counter++;
    });
    animTimer->start(4000);
    
    // ========================================================================
    // Show All Widgets
    // ========================================================================
    monitor->show();
    clockWidget->show();
    graphWidget->show();
    demo->show();
    
    // Cleanup on exit
    QObject::connect(&app, &QApplication::aboutToQuit, []() {
        cleanupAPIs();
    });
    
    return app.exec();
}
