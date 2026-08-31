cmake_minimum_required(VERSION 3.22)

project(ACModOrganize
    VERSION 1.0.0
    DESCRIPTION "Assetto Corsa Mod Brand & Country Organizer"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable Qt automated preprocessors (MOC, UIC)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt 6 libraries
find_package(Qt6 6.4 REQUIRED COMPONENTS
    Core
    Gui
    Quick
    Qml
    QuickControls2
    Svg
    Concurrent
    Network
)

qt_standard_project_setup(REQUIRES 6.4)

# Suppress QTP0004 warning
if(COMMAND qt_policy)
    qt_policy(SET QTP0004 NEW)
endif()

# Header search paths
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# nlohmann/json dependency handling
find_package(nlohmann_json 3.11 QUIET)
if(NOT nlohmann_json_FOUND)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/include/nlohmann/json.hpp")
        message(STATUS "Using bundled nlohmann/json from include/nlohmann/json.hpp")
    else()
        message(STATUS "Fetching nlohmann/json via FetchContent...")
        include(FetchContent)
        FetchContent_Declare(
            nlohmann_json
            GIT_REPOSITORY https://github.com/nlohmann/json.git
            GIT_TAG v3.11.3
        )
        FetchContent_MakeAvailable(nlohmann_json)
    endif()
endif()

# Source and header files
set(PROJECT_SOURCES
    src/main.cpp
    src/CarItem.cpp
    src/CarListModel.cpp
    src/BrandDetector.cpp
    src/ScannerEngine.cpp
    src/JsonWriter.cpp
    src/AppController.cpp
    src/UpdateManager.cpp
    src/Logger.cpp
)

set(PROJECT_HEADERS
    include/CarItem.hpp
    include/CarListModel.hpp
    include/BrandDetector.hpp
    include/ScannerEngine.hpp
    include/JsonWriter.hpp
    include/AppController.hpp
    include/UpdateManager.hpp
    include/Logger.hpp
)

# Create application executable
qt_add_executable(ACModOrganize
    ${PROJECT_SOURCES}
    ${PROJECT_HEADERS}
    app_icon.rc
)

set_source_files_properties(ui/Theme.qml PROPERTIES
    QT_QML_SINGLETON_TYPE TRUE
)

# Modern Qt 6 QML module integration (automatically compiles and embeds all QML & SVG assets)
qt_add_qml_module(ACModOrganize
    URI ACBO
    VERSION 1.0
    RESOURCE_PREFIX "/qt/qml"
    QML_FILES
        ui/Theme.qml
        ui/Main.qml
        ui/TopBar.qml
        ui/NavigationRail.qml
        ui/CarCard.qml
        ui/CarTableView.qml
        ui/UpdateNotificationDialog.qml
    RESOURCES
        ui/icons/car.svg
        ui/icons/car_white.svg
        ui/icons/folder.svg
        ui/icons/folder_white.svg
        ui/icons/search.svg
        ui/icons/search_white.svg
        ui/icons/zap.svg
        ui/icons/zap_white.svg
        ui/icons/save.svg
        ui/icons/save_white.svg
        ui/icons/save_dark.svg
        ui/icons/check.svg
        ui/icons/check_white.svg
        ui/icons/alert.svg
        ui/icons/alert_white.svg
        ui/icons/edit.svg
        ui/icons/edit_white.svg
        ui/icons/close.svg
        ui/icons/close_white.svg
        ui/icons/shield.svg
        ui/icons/shield_white.svg
        ui/icons/sun.svg
        ui/icons/sun_white.svg
        ui/icons/moon.svg
        ui/icons/moon_white.svg
        ui/icons/grid.svg
        ui/icons/grid_white.svg
        ui/icons/table.svg
        ui/icons/table_white.svg
        ui/icons/refresh.svg
        ui/icons/refresh_dark.svg
        ui/icons/refresh_white.svg
        ui/icons/copy.svg
        ui/icons/copy_white.svg
        ui/icons/app_icon.svg
        ui/icons/app_icon.ico
        ui/icons/open.svg
        ui/icons/open_white.svg
)

# Link dependencies
target_link_libraries(ACModOrganize PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
    Qt6::Qml
    Qt6::QuickControls2
    Qt6::Svg
    Qt6::Concurrent
    Qt6::Network
)

if(TARGET nlohmann_json::nlohmann_json)
    target_link_libraries(ACModOrganize PRIVATE nlohmann_json::nlohmann_json)
endif()

# Compiler options
if(MSVC)
    target_compile_options(ACModOrganize PRIVATE
        /W4
        /utf-8
        /permissive-
        /EHsc
        /Zc:__cplusplus
    )
else()
    target_compile_options(ACModOrganize PRIVATE
        -Wall
        -Wextra
        -pedantic
    )
endif()

# Output settings - WIN32_EXECUTABLE removes CMD console on Windows!
set_target_properties(ACModOrganize PROPERTIES
    OUTPUT_NAME "ACModOrganize"
    VS_STARTUP_PROJECT ACModOrganize
    WIN32_EXECUTABLE TRUE
)

# Root 1-click launcher executable
add_executable(ACBO WIN32
    src/launcher.cpp
    app_icon.rc
)
set_target_properties(ACBO PROPERTIES
    OUTPUT_NAME "ACBO"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}"
)

# Test suite target
option(BUILD_TESTS "Build backend engine test suite" ON)
if(BUILD_TESTS)
    add_executable(acbo_tests
        tests/test_backend.cpp
        src/CarItem.cpp
        src/BrandDetector.cpp
        src/ScannerEngine.cpp
        src/JsonWriter.cpp
        src/UpdateManager.cpp
        src/Logger.cpp
        include/Logger.hpp
        include/CarItem.hpp
        include/BrandDetector.hpp
        include/ScannerEngine.hpp
        include/JsonWriter.hpp
        include/UpdateManager.hpp
    )
    target_link_libraries(acbo_tests PRIVATE
        Qt6::Core
        Qt6::Concurrent
        Qt6::Network
    )
    if(TARGET nlohmann_json::nlohmann_json)
        target_link_libraries(acbo_tests PRIVATE nlohmann_json::nlohmann_json)
    endif()
    if(MSVC)
        target_compile_options(acbo_tests PRIVATE /W4 /utf-8 /permissive- /EHsc /Zc:__cplusplus)
    endif()
endif()
