cmake_minimum_required(VERSION 3.10.0)

project(Dreamnamer)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Widgets REQUIRED)

add_executable(Dreamnamer
    MainWindow.ui
    MainWindow.cpp
    main.cpp
)

target_link_libraries(Dreamnamer Qt5::Widgets)
