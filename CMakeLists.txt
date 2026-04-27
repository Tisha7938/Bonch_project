cmake_minimum_required(VERSION 3.16)
project(final_project LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_SOURCE_DIR}/src/ui)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

set(CORE_SOURCES
        src/core/model/graph.cpp
        src/core/model/node.cpp
        src/core/model/edge.cpp
)
set(CORE_HEADERS
        src/core/model/graph.h
        src/core/model/node.h
        src/core/model/edge.h
        src/core/enums/graphenums.h
)

set(UI_SOURCES
        src/main.cpp
        src/ui/mainwindow.cpp
        src/ui/graphwidget.cpp
)
set(UI_HEADERS
        src/ui/mainwindow.h
        src/ui/graphwidget.h
)
set(UI_FORMS
        src/ui/mainwindow.ui
)

set(RESOURCES
        resources/icons
)


add_executable(final_project
        ${CORE_SOURCES}
        ${CORE_HEADERS}
        ${UI_SOURCES}
        ${UI_HEADERS}
        ${UI_FORMS}
        ${RESOURCES}
        src/core/model/nodeinfowidget.h src/core/model/nodeinfowidget.cpp
)

target_link_libraries(final_project PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
)

target_include_directories(final_project PRIVATE
        ${CMAKE_SOURCE_DIR}/src/core/model
        ${CMAKE_SOURCE_DIR}/src/core/enums
        ${CMAKE_SOURCE_DIR}/src/ui
)