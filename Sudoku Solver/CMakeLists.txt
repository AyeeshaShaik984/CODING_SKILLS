cmake_minimum_required(VERSION 3.16)
project(SudokuSolver LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)  # already works for you

qt_standard_project_setup()    # if you created from Qt template; else see below

qt_add_executable(SudokuSolver
    src/main.cpp
    src/MainWindow.cpp
    src/MainWindow.h
    src/SudokuEngine.cpp
    src/SudokuEngine.h
)

target_link_libraries(SudokuSolver PRIVATE Qt6::Widgets)
