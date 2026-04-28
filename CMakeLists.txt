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
        src/core/model/messagebus.cpp
        src/core/model/nodemodel.cpp
)
set(CORE_HEADERS
        src/core/model/messagebus.h
        src/core/model/nodemodel.h
)

set(UI_SOURCES
        src/main.cpp
        src/ui/mainwindow.cpp
        src/ui/graphwidget.cpp
        src/ui/graph.cpp
        src/ui/node.cpp
        src/ui/edge.cpp
        src/ui/nodeinfowidget.cpp
)
set(UI_HEADERS
        src/ui/mainwindow.h
        src/ui/graphwidget.h
        src/ui/graph.h
        src/ui/node.h
        src/ui/edge.h
        src/ui/nodeinfowidget.h
        src/core/enums/graphenums.h
)
set(UI_FORMS
        src/ui/mainwindow.ui
)

set(RESOURCES
        resources/icons
)

set(STRATEGIES_HEADERS
        src/core/strategies/imaintenancestrategy.h
        src/core/strategies/strategybasiccontrol.h
        src/core/strategies/strategyinstantdetection.h
        src/core/strategies/strategypreventivewithcontrol.h
        src/core/strategies/strategynocontrolcheck.h
        src/core/strategies/strategyinstantcheckdetection.h
        src/core/strategies/strategyofflinecheck.h
        src/core/strategies/strategyfixedintervalcontrol.h
        src/core/strategies/strategyperiodiccontrolemergencyrecovery.h
)

set(STRATEGIES_SOURCES
        src/core/strategies/strategybasiccontrol.cpp
        src/core/strategies/strategyinstantdetection.cpp
        src/core/strategies/strategypreventivewithcontrol.cpp
        src/core/strategies/strategynocontrolcheck.cpp
        src/core/strategies/strategyinstantcheckdetection.cpp
        src/core/strategies/strategyofflinecheck.cpp
        src/core/strategies/strategyfixedintervalcontrol.cpp
        src/core/strategies/strategyperiodiccontrolemergencyrecovery.cpp
)

set(SIMULATION_HEADERS
        src/core/simulation/neuralsystememulator.h
        src/core/simulation/systemfromneuralmodel.h
        src/core/simulation/simulationengine.h
)

set(SIMULATION_SOURCES
        src/core/simulation/neuralsystememulator.cpp
        src/core/simulation/systemfromneuralmodel.cpp
        src/core/simulation/simulationengine.cpp
)

set(UTILS
    src/utils/logger.h
)

add_executable(final_project
        ${CORE_SOURCES}
        ${CORE_HEADERS}
        ${UI_SOURCES}
        ${UI_HEADERS}
        ${UI_FORMS}
        ${RESOURCES}
        ${SIMULATION_HEADERS}
        ${SIMULATION_SOURCES}
        ${STRATEGIES_HEADERS}
        ${STRATEGIES_SOURCES}
        ${UTILS}
)

target_link_libraries(final_project PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
)

target_include_directories(final_project PRIVATE
        ${CMAKE_SOURCE_DIR}/src/core/model
        ${CMAKE_SOURCE_DIR}/src/core/enums
        ${CMAKE_SOURCE_DIR}/src/core/strategies
        ${CMAKE_SOURCE_DIR}/src/core/simulation
        ${CMAKE_SOURCE_DIR}/src/ui
        ${CMAKE_SOURCE_DIR}/src/utils
)