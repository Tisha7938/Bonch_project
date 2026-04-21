cmake_minimum_required(VERSION 3.16)
project(final_project)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_AUTOUIC_SEARCH_PATHS include src .)

find_package(Qt6 COMPONENTS
        Core
        Gui
        Widgets
        REQUIRED)

add_executable(final_project
        src/main.cpp
        src/edge.cpp
        src/graph.cpp
        src/graphwidget.cpp
        src/mainwindow.cpp
        src/node.cpp
        include/edge.h
        include/graph.h
        include/graphenums.h
        include/graphwidget.h
        include/mainwindow.h
        include/node.h
        mainwindow.ui
        .gitignore
)

target_link_libraries(final_project
        Qt::Core
        Qt::Gui
        Qt::Widgets
)
