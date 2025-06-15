cmake_minimum_required(VERSION 3.31)
project(cpppython)

set(CMAKE_CXX_STANDARD 20)

# Enable optimization flags
if(MSVC)
    # Microsoft Visual C++
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /Ob2 /Oi /Ot /GL")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG")
else()
    # GCC/Clang
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native -flto")
    if(APPLE)
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -mtune=native")
    endif()
endif()

# Enable Link Time Optimization
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find required packages
find_package(Qt5 COMPONENTS
        Core
        Widgets
        Charts
        PrintSupport
        REQUIRED
)

find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
find_package(pybind11 REQUIRED)

# ============================================================================
# LAYERED ARCHITECTURE IMPLEMENTATION
# ============================================================================

# 1. QCustomPlot Library (Base plotting functionality)
set(QCUSTOMPLOT_SOURCES
        classes/qcustomplot.cpp
        classes/qcustomplot.h
        classes/qcustomplot_wrapper.h
)

add_library(qcustomplot STATIC ${QCUSTOMPLOT_SOURCES})
target_link_libraries(qcustomplot Qt5::Core Qt5::Widgets Qt5::PrintSupport)
target_compile_definitions(qcustomplot PUBLIC QCUSTOMPLOT_USE_LIBRARY)

# 2. Qt Implementation Layer (Qt-specific functionality with MOC)
set(QT_IMPL_SOURCES
        classes/PlotWidgetImpl.cpp
        classes/PlotWidgetImpl.h
)

add_library(qt_impl STATIC ${QT_IMPL_SOURCES})
target_link_libraries(qt_impl
        Qt5::Core
        Qt5::Widgets
        Qt5::Charts
        Qt5::PrintSupport
        qcustomplot
)
# Add QT_NO_KEYWORDS to fix signals conflict
target_compile_definitions(qt_impl PRIVATE QT_NO_KEYWORDS)

# 3. Wrapper Layer (Qt-free interface, NO Qt headers, NO MOC)
set(WRAPPER_SOURCES
        classes/PlotWidgetWrapper.cpp
        classes/PlotWidgetWrapper.h
)

add_library(plot_wrapper STATIC ${WRAPPER_SOURCES})
target_link_libraries(plot_wrapper qt_impl)
# Add QT_NO_KEYWORDS to be safe
target_compile_definitions(plot_wrapper PRIVATE QT_NO_KEYWORDS)
# Disable AutoMOC for wrapper layer since it should be Qt-free
set_target_properties(plot_wrapper PROPERTIES AUTOMOC OFF)

# 4. Python Module (Only links to wrapper, isolated from Qt)
pybind11_add_module(plot_module classes/pybind11_bindings.cpp)
target_link_libraries(plot_module PRIVATE plot_wrapper)

# Platform-specific settings for Python module
if(WIN32)
    target_compile_definitions(plot_module PRIVATE WIN32_LEAN_AND_MEAN)
endif()

# ============================================================================
# MAIN APPLICATION
# ============================================================================

add_executable(cpppython
        main.cpp
        classes/MainWindow.cpp
        classes/MainWindow.h
        classes/QtOutputBuffer.cpp
        classes/QtOutputBuffer.h
        classes/PythonEngine.cpp
        classes/PythonEngine.h
        classes/DataAnalysisApp.cpp
        classes/DataAnalysisApp.h
        classes/PythonHighlighter.cpp
        classes/PythonHighlighter.h
)

target_link_libraries(cpppython
        Qt5::Core
        Qt5::Widgets
        Qt5::Charts
        Qt5::PrintSupport
        plot_wrapper  # Use the wrapper instead of direct Qt libs
        pybind11::embed
        Python3::Python
)

# Fix Python/Qt slot conflict
target_compile_definitions(cpppython PRIVATE QT_NO_KEYWORDS)

# Platform-specific configurations
if(WIN32)
    # Prevent console window on Windows
    set_target_properties(cpppython PROPERTIES
            AUTOMOC ON
            WIN32_EXECUTABLE TRUE
    )
endif()

# For macOS app bundles
if(APPLE)
    set_target_properties(cpppython PROPERTIES
            AUTOMOC ON
            MACOSX_BUNDLE TRUE
            MACOSX_BUNDLE_GUI_IDENTIFIER "com.example.QtPlotExample"
            MACOSX_BUNDLE_BUNDLE_VERSION "1.0"
            MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0"
    )
endif()