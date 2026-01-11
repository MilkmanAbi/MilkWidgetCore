/**
 * MilkWidgetCore - A Conky-level hyper-easy widget engine
 * 
 * Main public header - include this to get everything
 * 
 * Usage:
 *   #include <milk/MilkWidget.h>
 *   using namespace Milk;
 *   
 *   int main(int argc, char** argv) {
 *       Application app(argc, argv);
 *       Widget* w = Widget::fromFile("mywidget.xml");
 *       w->show();
 *       return app.exec();
 *   }
 */

#pragma once

#define MILK_VERSION_MAJOR 1
#define MILK_VERSION_MINOR 0
#define MILK_VERSION_PATCH 0
#define MILK_VERSION_STRING "1.0.0"

#include "Types.h"
#include "Widget.h"
#include "Application.h"
#include "Widgets.h"
#include "APIs.h"
#include "Parsers.h"
#include "Utils.h"

namespace Milk {

void init();
void cleanup();
const char* version();

// Quick creation helpers
Widget* widget(int w = 300, int h = 200);
Widget* circle(int diameter = 100);
Widget* square(int size = 100);

// Load from file
Widget* load(const QString&amp; xmlPath);
Widget* loadTheme(const QString&amp; themeName);

} // namespace Milk
