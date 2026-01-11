cmake_minimum_required(VERSION 3.16)

# System Monitor Example
add_executable(milk_system_monitor
    system_monitor/main.cpp
)

target_link_libraries(milk_system_monitor PRIVATE
    MilkWidgetCore
)

# XML Widget Example
add_executable(milk_xml_demo
    xml_demo/main.cpp
)

target_link_libraries(milk_xml_demo PRIVATE
    MilkWidgetCore
)

# Install examples
install(TARGETS milk_system_monitor milk_xml_demo
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
