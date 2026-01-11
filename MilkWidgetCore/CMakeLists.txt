cmake_minimum_required(VERSION 3.16)
project(MilkWidgetCore 
    VERSION 1.0.0
    DESCRIPTION "A Conky-level hyper-easy widget engine for Linux/Windows"
    LANGUAGES CXX
)

# ============================================================================
# Build Options
# ============================================================================
option(MILK_BUILD_EXAMPLES "Build example applications" ON)
option(MILK_BUILD_CLI "Build command-line widget runner" ON)
option(MILK_PREFER_QT6 "Prefer Qt6 over Qt5 if both available" ON)
option(MILK_ENABLE_WAYLAND "Enable Wayland support (Linux)" ON)
option(MILK_ENABLE_X11 "Enable X11 support (Linux)" ON)

# ============================================================================
# C++ Standard
# ============================================================================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# ============================================================================
# Find Qt (prefer Qt6, fallback to Qt5)
# ============================================================================
set(QT_COMPONENTS Core Gui Widgets Network Xml)

if(MILK_PREFER_QT6)
    find_package(Qt6 QUIET COMPONENTS ${QT_COMPONENTS})
    if(Qt6_FOUND)
        set(QT_VERSION_MAJOR 6)
        message(STATUS "Found Qt6 ${Qt6_VERSION}")
    else()
        find_package(Qt5 5.15 REQUIRED COMPONENTS ${QT_COMPONENTS})
        set(QT_VERSION_MAJOR 5)
        message(STATUS "Found Qt5 ${Qt5_VERSION}")
    endif()
else()
    find_package(Qt5 5.15 QUIET COMPONENTS ${QT_COMPONENTS})
    if(Qt5_FOUND)
        set(QT_VERSION_MAJOR 5)
        message(STATUS "Found Qt5 ${Qt5_VERSION}")
    else()
        find_package(Qt6 REQUIRED COMPONENTS ${QT_COMPONENTS})
        set(QT_VERSION_MAJOR 6)
        message(STATUS "Found Qt6 ${Qt6_VERSION}")
    endif()
endif()

# Platform-specific Qt components
if(UNIX AND NOT APPLE)
    if(MILK_ENABLE_X11 AND QT_VERSION_MAJOR EQUAL 5)
        find_package(Qt5 COMPONENTS X11Extras QUIET)
        if(Qt5X11Extras_FOUND)
            list(APPEND QT_EXTRA_LIBS Qt5::X11Extras)
            add_compile_definitions(MILK_HAS_X11_EXTRAS)
        endif()
    endif()
    
    # Find X11
    if(MILK_ENABLE_X11)
        find_package(X11 QUIET)
        if(X11_FOUND)
            add_compile_definitions(MILK_HAS_X11)
        endif()
    endif()
endif()

# ============================================================================
# Library Sources
# ============================================================================
set(MILK_CORE_SOURCES
    src/core/Widget.cpp
    src/core/Application.cpp
)

set(MILK_WIDGET_SOURCES
    src/widgets/Widgets.cpp
)

set(MILK_API_SOURCES
    src/apis/SystemMonitor.cpp
    src/apis/APIs.cpp
)

set(MILK_PARSER_SOURCES
    src/parsers/XMLParser.cpp
    src/parsers/CSSParser.cpp
)

set(MILK_UTIL_SOURCES
    src/utils/Utils.cpp
)

set(MILK_HEADERS
    include/milk/MilkWidget.h
    include/milk/Widget.h
    include/milk/Application.h
    include/milk/Widgets.h
    include/milk/APIs.h
    include/milk/Parsers.h
    include/milk/Utils.h
    include/milk/Types.h
)

# ============================================================================
# Main Library
# ============================================================================
add_library(MilkWidgetCore SHARED
    ${MILK_CORE_SOURCES}
    ${MILK_WIDGET_SOURCES}
    ${MILK_API_SOURCES}
    ${MILK_PARSER_SOURCES}
    ${MILK_UTIL_SOURCES}
)

add_library(MilkWidget::Core ALIAS MilkWidgetCore)

target_include_directories(MilkWidgetCore
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link Qt libraries
if(QT_VERSION_MAJOR EQUAL 6)
    target_link_libraries(MilkWidgetCore PUBLIC
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Network
        Qt6::Xml
    )
else()
    target_link_libraries(MilkWidgetCore PUBLIC
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Qt5::Network
        Qt5::Xml
        ${QT_EXTRA_LIBS}
    )
endif()

# Platform-specific libraries
if(UNIX AND NOT APPLE)
    if(X11_FOUND)
        target_link_libraries(MilkWidgetCore PRIVATE ${X11_LIBRARIES})
        target_include_directories(MilkWidgetCore PRIVATE ${X11_INCLUDE_DIR})
    endif()
endif()

# Compiler definitions
target_compile_definitions(MilkWidgetCore PRIVATE
    MILK_LIBRARY
    QT_VERSION_MAJOR=${QT_VERSION_MAJOR}
)

# Library properties
set_target_properties(MilkWidgetCore PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    EXPORT_NAME Core
    OUTPUT_NAME milkwidget
)

# ============================================================================
# Static Library (optional)
# ============================================================================
add_library(MilkWidgetCore_static STATIC
    ${MILK_CORE_SOURCES}
    ${MILK_WIDGET_SOURCES}
    ${MILK_API_SOURCES}
    ${MILK_PARSER_SOURCES}
    ${MILK_UTIL_SOURCES}
)

target_include_directories(MilkWidgetCore_static
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

if(QT_VERSION_MAJOR EQUAL 6)
    target_link_libraries(MilkWidgetCore_static PUBLIC
        Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Network Qt6::Xml
    )
else()
    target_link_libraries(MilkWidgetCore_static PUBLIC
        Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Network Qt5::Xml
        ${QT_EXTRA_LIBS}
    )
endif()

target_compile_definitions(MilkWidgetCore_static PRIVATE
    MILK_STATIC
    QT_VERSION_MAJOR=${QT_VERSION_MAJOR}
)

set_target_properties(MilkWidgetCore_static PROPERTIES
    OUTPUT_NAME milkwidget_static
)

# ============================================================================
# CLI Widget Runner
# ============================================================================
if(MILK_BUILD_CLI)
    add_executable(milkwidget-cli
        src/main_cli.cpp
    )
    
    target_link_libraries(milkwidget-cli PRIVATE MilkWidgetCore)
    
    set_target_properties(milkwidget-cli PROPERTIES
        OUTPUT_NAME "milkwidget"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
endif()

# ============================================================================
# Examples
# ============================================================================
if(MILK_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# ============================================================================
# Installation
# ============================================================================
include(GNUInstallDirs)

install(TARGETS MilkWidgetCore MilkWidgetCore_static
    EXPORT MilkWidgetTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/milk
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

if(MILK_BUILD_CLI)
    install(TARGETS milkwidget-cli
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()

# Install themes
install(DIRECTORY themes/
    DESTINATION ${CMAKE_INSTALL_DATADIR}/milkwidget/themes
)

# Export targets
install(EXPORT MilkWidgetTargets
    FILE MilkWidgetTargets.cmake
    NAMESPACE MilkWidget::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/MilkWidget
)

# ============================================================================
# Summary
# ============================================================================
message(STATUS "")
message(STATUS "MilkWidgetCore Configuration Summary:")
message(STATUS "  Version:        ${PROJECT_VERSION}")
message(STATUS "  Qt Version:     ${QT_VERSION_MAJOR}")
message(STATUS "  Build Examples: ${MILK_BUILD_EXAMPLES}")
message(STATUS "  Build CLI:      ${MILK_BUILD_CLI}")
if(UNIX AND NOT APPLE)
    message(STATUS "  X11 Support:    ${X11_FOUND}")
    message(STATUS "  Wayland:        ${MILK_ENABLE_WAYLAND}")
endif()
message(STATUS "")
