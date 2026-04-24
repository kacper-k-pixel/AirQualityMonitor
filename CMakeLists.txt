cmake_minimum_required(VERSION 3.20)
project(AirQualityMonitor VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# ── Qt 6 ──────────────────────────────────────────────────────────────────────
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network Charts)

# ── nlohmann/json (header-only, pobierany przy konfiguracji) ──────────────────
include(FetchContent)
FetchContent_Declare(nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3)
set(JSON_BuildTests OFF CACHE INTERNAL "")
set(JSON_Install    OFF CACHE INTERNAL "")
FetchContent_MakeAvailable(nlohmann_json)

# ── Źródła aplikacji ──────────────────────────────────────────────────────────
set(APP_SOURCES
    src/main.cpp
    src/api/RestClient.cpp
    src/api/JsonParser.cpp
    src/core/DataController.cpp
    src/core/Analyzer.cpp
    src/storage/JsonDatabase.cpp
    src/gui/MainWindow.cpp
    src/gui/ChartWidget.cpp
    src/gui/MapWidget.cpp
    src/gui/AnalysisWidget.cpp
)

# ── Zasoby Qt (mapa Polski PNG) ───────────────────────────────────────────────
set(APP_RESOURCES
    src/gui/resources.qrc
)

add_executable(AirQualityMonitor ${APP_SOURCES} ${APP_RESOURCES})

target_include_directories(AirQualityMonitor PRIVATE src)

target_link_libraries(AirQualityMonitor PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Qt6::Charts
    nlohmann_json::nlohmann_json
)

# ── Testy jednostkowe ─────────────────────────────────────────────────────────
enable_testing()
add_subdirectory(tests)