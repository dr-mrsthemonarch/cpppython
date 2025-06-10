cmake_minimum_required(VERSION 3.31)
project(qt_esting)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)


find_package(Qt5 COMPONENTS
        Core
        Widgets
        Charts
        REQUIRED
)

find_package(Python3 COMPONENTS Interpreter Development REQUIRED
)

# Find pybind11
find_package(pybind11 QUIET)

add_executable(qt_esting main.cpp)
target_link_libraries(qt_esting
        Qt5::Core
        Qt5::Widgets
        Qt5::Charts
        pybind11::embed
        Python3::Python
)
# Fix Python/Qt slot conflict
target_compile_definitions(qt_esting PRIVATE QT_NO_KEYWORDS)
# Platform-specific configurations
if(WIN32)
    # Prevent console window on Windows
    set_target_properties(qt_esting PROPERTIES
            AUTOMOC ON
            WIN32_EXECUTABLE TRUE
    )
endif()

# For macOS app bundles
if(APPLE)
    set_target_properties(qt_esting PROPERTIES
            AUTOMOC ON
            MACOSX_BUNDLE TRUE
            MACOSX_BUNDLE_GUI_IDENTIFIER "com.example.QtPlotExample"
            MACOSX_BUNDLE_BUNDLE_VERSION "1.0"
            MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0"
    )
endif()
