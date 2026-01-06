cmake_minimum_required(VERSION 3.10)

project(Dreamnamer)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Widgets Network REQUIRED)
find_package(Boost)

set(MACOSX_BUNDLE_ICON_FILE dreamnamer.icns)
set(APP_ICON_PNG ${CMAKE_SOURCE_DIR}/dreamnamer.png)
set(APP_ICON_ICNS ${CMAKE_BINARY_DIR}/dreamnamer.icns)
set(ICONSET_DIR ${CMAKE_BINARY_DIR}/dreamnamer.iconset)

add_custom_command(
  OUTPUT ${APP_ICON_ICNS}
  COMMAND ${CMAKE_COMMAND} -E make_directory ${ICONSET_DIR}
  COMMAND sips -z 16 16 ${APP_ICON_PNG} --out ${ICONSET_DIR}/icon_16x16.png
  COMMAND sips -z 32 32 ${APP_ICON_PNG} --out ${ICONSET_DIR}/icon_16x16@2x.png
  COMMAND sips -z 32 32 ${APP_ICON_PNG} --out ${ICONSET_DIR}/icon_32x32.png
  COMMAND sips -z 64 64 ${APP_ICON_PNG} --out ${ICONSET_DIR}/icon_32x32@2x.png
  COMMAND sips -z 128 128 ${APP_ICON_PNG} --out ${ICONSET_DIR}/icon_128x128.png
  COMMAND sips -z 256 256 ${APP_ICON_PNG} --out ${ICONSET_DIR}/icon_128x128@2x.png
  COMMAND sips -z 256 256 ${APP_ICON_PNG} --out ${ICONSET_DIR}/icon_256x256.png
  COMMAND sips -z 512 512 ${APP_ICON_PNG} --out ${ICONSET_DIR}/icon_256x256@2x.png
  COMMAND sips -z 512 512 ${APP_ICON_PNG} --out ${ICONSET_DIR}/icon_512x512.png
  COMMAND sips -z 1024 1024 ${APP_ICON_PNG} --out ${ICONSET_DIR}/icon_512x512@2x.png
  COMMAND iconutil -c icns ${ICONSET_DIR} -o ${APP_ICON_ICNS}
  COMMAND ${CMAKE_COMMAND} -E remove_directory ${ICONSET_DIR}
  DEPENDS ${APP_ICON_PNG}
  COMMENT "Generating icns from PNG"
)

set_source_files_properties(${APP_ICON_ICNS} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

add_executable(${PROJECT_NAME}
  MACOSX_BUNDLE
  MainWindow.ui
  MainWindow.cpp
  MovieDatabaseQuery.cpp
  Index.cpp
  main.cpp
  ${APP_ICON_ICNS}
)


set_target_properties(${PROJECT_NAME} PROPERTIES
  MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/Info.plist.in)

option(ENABLE_TEST "" OFF)
add_subdirectory(LucenePlusPlus)
include_directories(LucenePlusPlus/include ${CMAKE_BINARY_DIR}/LucenePlusPlus/include)

set_property(TARGET ${PROJECT_NAME}  PROPERTY CXX_STANDARD 20)

target_link_libraries(${PROJECT_NAME} Qt6::Widgets Qt6::Network lucene++)
