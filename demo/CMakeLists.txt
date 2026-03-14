cmake_minimum_required(VERSION 3.10)
project(dso_ui)

find_package(raylib 4.5 QUIET)

set(DSO_TOP ${CMAKE_CURRENT_SOURCE_DIR}/../)
set(DSO_SRC ${DSO_TOP}/src)
set(UI_FOLDER ${DSO_TOP}/src/ui)

add_definitions("-g3")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
SET(ILI9341_DRIVER  ${CMAKE_CURRENT_SOURCE_DIR}/ili_ray.cpp)
include_directories(../externals/simplerILI9341/cpp)
ADD_SUBDIRECTORY(../externals/simplerILI9341/cpp ili)
target_include_directories(simplerILI9341 PRIVATE ${UI_FOLDER} ${DSO_SRC} ${DSO_TOP} ${DSO_TOP}/include foo)



add_executable(${PROJECT_NAME} )
target_sources(${PROJECT_NAME} PRIVATE main.cpp ili_ray.cpp stubs.cpp assets.cpp
      ${UI_FOLDER}/dso_menuEngine.cpp
      ${DSO_SRC}/dso_gfx.cpp ${DSO_SRC}/dso_display.cpp
      ${DSO_TOP}/assets/gfx.cpp)
target_include_directories(${PROJECT_NAME} PRIVATE ${UI_FOLDER} ${DSO_SRC} ${DSO_TOP} ${DSO_TOP}/include foo)


target_link_libraries(${PROJECT_NAME} simplerILI9341 raylib m pthread dl rt X11)
