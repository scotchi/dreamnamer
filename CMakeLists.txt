cmake_minimum_required(VERSION 3.10)

project(Dreamnamer)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Widgets REQUIRED)

add_executable(${PROJECT_NAME}
  MACOSX_BUNDLE
  MainWindow.ui
  MainWindow.cpp
  Index.cpp
  main.cpp
)

add_subdirectory(LucenePlusPlus)
include_directories(LucenePlusPlus/include ${CMAKE_BINARY_DIR}/LucenePlusPlus/include)

set_property(TARGET ${PROJECT_NAME}  PROPERTY CXX_STANDARD 17)

target_link_libraries(${PROJECT_NAME} Qt5::Widgets lucene++)
