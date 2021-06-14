cmake_minimum_required(VERSION 3.10)

project(Dreamnamer)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Widgets Network REQUIRED)
find_package(Boost)

add_executable(${PROJECT_NAME}
  MACOSX_BUNDLE
  MainWindow.ui
  MainWindow.cpp
  MovieDatabaseQuery.cpp
  Index.cpp
  main.cpp
)


set_target_properties(${PROJECT_NAME} PROPERTIES
  MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/Info.plist.in)

option(ENABLE_TEST "" OFF)
add_subdirectory(LucenePlusPlus)
include_directories(LucenePlusPlus/include ${CMAKE_BINARY_DIR}/LucenePlusPlus/include)

set_property(TARGET ${PROJECT_NAME}  PROPERTY CXX_STANDARD 20)

target_link_libraries(${PROJECT_NAME} Qt6::Widgets Qt6::Network lucene++)
