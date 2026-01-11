# MilkWidgetCore

A Conky-level hyper-easy desktop widget engine for Linux (cross-platform via Qt5/6).

## Features

- **Simple C++ API** - Create widgets in just a few lines
- **XML Configuration** - Define widgets declaratively
- **CSS Styling** - Style widgets with familiar CSS syntax
- **System Monitoring** - Built-in CPU, memory, disk, network, battery APIs
- **Animations** - Fade, bounce, pulse, shake, scale effects
- **Modern Effects** - Glass blur, shadows, glow, gradients
- **Cross-Platform** - Linux primary, Windows/macOS via Qt

## Quick Start

```cpp
#include <milk/MilkWidget.h>
using namespace Milk;

int main(int argc, char *argv[]) {
    Application app(argc, argv);
    
    // Create a widget
    Widget* w = Widget::create(300, 200);
    w->setBackground(30, 35, 45, 220);
    w->setRounded(12);
    w->setPosition(Position::TopRight);
    w->setGlass(true);
    
    // Add content
    Text* t = title("System Monitor", w);
    t->setColor("#4A9EFF");
    
    ProgressBar* cpu = progressBar(w);
    cpu->setColors("#333", "#FF6B6B");
    
    // Update from system
    SystemMonitor* sys = SystemMonitor::instance();
    w->onUpdate([=]() {
        cpu->setValue(sys->cpu());
    });
    w->setUpdateInterval(1000);
    
    w->show();
    return app.exec();
}
```

## XML Configuration

```xml
<widget width="320" height="200" 
        background="rgba(30,35,45,220)" 
        rounded="10" 
        position="top-right"
        glass="true">
    
    <title color="#4A9EFF">System Monitor</title>
    
    <label color="#888">CPU Usage</label>
    <progress id="cpu" bg="#333" color="#FF6B6B"/>
    
    <label color="#888">Memory</label>
    <progress id="mem" bg="#333" color="#4ECDC4"/>
</widget>
```

## Building

### Requirements
- Qt 5.15+ or Qt 6.x
- CMake 3.16+
- C++17 compiler

### Build Steps

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
sudo make install
```

### CMake Options
- `MILK_BUILD_EXAMPLES` - Build examples (ON)
- `MILK_BUILD_CLI` - Build CLI tool (ON)
- `MILK_PREFER_QT6` - Prefer Qt6 (ON)

## Widget Types

| Widget | Description |
|--------|-------------|
| `Widget` | Base container with effects |
| `Text` | Rich text display |
| `ProgressBar` | Animated progress bars |
| `Graph` | Line/area/bar charts |
| `Gauge` | Arc/circle gauges |
| `Image` | Image display with effects |
| `Button` | Interactive buttons |
| `Clock` | Digital/analog clocks |
| `Calendar` | Interactive calendar |
| `Container` | Layout containers |

## System APIs

| API | Data |
|-----|------|
| `SystemMonitor` | CPU, memory, disk, temp, processes |
| `NetworkMonitor` | Speed, totals, interfaces, IP |
| `BatteryMonitor` | Level, charging status |
| `WeatherAPI` | OpenWeatherMap integration |
| `MediaPlayer` | MPRIS media control |

## Positioning

```cpp
widget->setPosition(Position::TopRight);
widget->setPosition(Position::Center);
widget->setPosition(Position::BottomLeft);
// etc.
```

## Effects

```cpp
widget->setGlass(true);           // Frosted glass
widget->setBlur(BlurMode::Glass); // Background blur
widget->setGlow("#FF0000", 10);   // Outer glow
widget->setShadow(5, 5, 10);      // Drop shadow
```

## Animations

```cpp
widget->fadeIn(500);
widget->bounce(800);
widget->pulse(1000);
widget->shake(500, 5);
widget->scaleTo(1.2, 300);
```

## License

MIT License - See LICENSE file

## Credits

Inspired by Conky, created for modern Linux desktops.
