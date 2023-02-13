#-----------------------------------------------------------------------------
#
# CMakeLists for lnDSO
# An opensource firmware for the DSO SHELL 150
# With optional hardware mods :
#   - Alternative rotary encoder wiring : MUCH Better, almost mandatory
#
#-----------------------------------------------------------------------------
PROJECT(pb_test NONE) # the NONE is very important !
SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)
cmake_minimum_required(VERSION 3.0)

enable_language(C CXX ) # this is important too!

MESSAGE(STATUS "===============")
MESSAGE(STATUS "Starting DSO_pb")
MESSAGE(STATUS "===============")
#____________________
# Options
#____________________
SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)
find_package(Qt6 REQUIRED COMPONENTS Core Network)
qt_standard_project_setup()
#
ADD_DEFINITIONS("-g3")
#
SET(PB ${CMAKE_CURRENT_SOURCE_DIR}/../pb_c)
SET(STUBS ${CMAKE_CURRENT_SOURCE_DIR}/stubs)

include_directories( ${PB} )
include_directories( ${STUBS} )
include_directories(../../externals/nanopb)
include_directories(../../src)

ADD_LIBRARY(gen_pb STATIC ${PB}/messaging.pb.cpp)
#
ADD_SUBDIRECTORY(../../externals/nanopb nanopb)

#
ADD_EXECUTABLE(cpp main.cpp ../../src/dso_usbd.cpp pseudo_usb.cpp api_stubs.cpp)
TARGET_LINK_LIBRARIES(cpp gen_pb nanopb)
TARGET_LINK_LIBRARIES(cpp Qt6::Network Qt6::Core)


MESSAGE(STATUS "\tFinal name is ${EXTENSION}")
