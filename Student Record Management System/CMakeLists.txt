cmake_minimum_required(VERSION 3.16)
project(StudentRecordManager LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Sql)

set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/startdialog.cpp
    src/logindialog.cpp
)

set(HEADERS
    src/mainwindow.h
    src/startdialog.h
    src/logindialog.h
)

set(RESOURCES
    resources.qrc
)

qt_add_executable(StudentRecordManager
    ${SOURCES}
    ${HEADERS}
    ${RESOURCES}
)

target_link_libraries(StudentRecordManager
    PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Sql
)
