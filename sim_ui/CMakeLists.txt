cmake_minimum_required(VERSION 3.10)
project(dso_ui)

find_package(raylib 4.5 QUIET)

set(DSO_TOP ${CMAKE_CURRENT_SOURCE_DIR}/../)
set(DSO_SRC ${DSO_TOP}/src)
set(DSO_ASSETS ${DSO_TOP}/assets)
set(UI_FOLDER ${DSO_TOP}/src/ui)
set(ESPRIT_TOP ${DSO_TOP}/esprit )
set(ESPRIT_ROOT ${ESPRIT_TOP})

if(true)
  SET(SIMPLER_ILI9341_FONT_COMPRESSION ON CACHE INTERNAL "")
  SET(SIMPLER_ILI9341_2BPP_SUPPORT     ON CACHE INTERNAL "")
else()
  SET(SIMPLER_ILI9341_FONT_COMPRESSION OFF CACHE INTERNAL "")
  SET(SIMPLER_ILI9341_2BPP_SUPPORT     OFF CACHE INTERNAL "")
  set(LOW_FONT TRUE)
  add_definitions("-DUSE_SMALLFOOTPRINT")
endif()

add_definitions("-g3 -O0")

include_directories(${DSO_TOP}/include)
include_directories(${DSO_TOP}/)
include_directories(${DSO_TOP}/src)
include_directories(${DSO_TOP}/src/ui)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
SET(ILI9341_DRIVER  ${CMAKE_CURRENT_SOURCE_DIR}/ili_ray.cpp)
include_directories(../externals/simplerILI9341/cpp)
ADD_SUBDIRECTORY(../externals/simplerILI9341/cpp ili)
target_include_directories(simplerILI9341 PRIVATE ${UI_FOLDER} ${DSO_SRC} ${DSO_TOP} ${DSO_TOP}/include foo)



add_executable(${PROJECT_NAME} )
target_sources(${PROJECT_NAME} PRIVATE
      main.cpp
      ili_ray.cpp
      stubs.cpp
      assets.cpp
      dso_control.cpp
      ${UI_FOLDER}/dso_menuEngine.cpp
      ${DSO_SRC}/dso_gfx.cpp
      ${DSO_SRC}/dso_display.cpp
      ${DSO_TOP}/assets/gfx.cpp
      ${DSO_ASSETS}/fonts.cpp
      ${DSO_SRC}/dso_settingMenu.cpp
      ${DSO_SRC}/ui/dso_menuEngine.cpp
)
add_library(esprit_dev INTERFACE)
if(LOW_FONT )
  target_compile_definitions(${PROJECT_NAME} PRIVATE LOW_FONT=1)
endif()
include(${ESPRIT_TOP}/cmake/lnEmbeddedPrint.cmake)


target_link_libraries(${PROJECT_NAME} embeddedPrintf simplerILI9341 raylib m pthread dl rt X11)
