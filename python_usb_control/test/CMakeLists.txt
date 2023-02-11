#-----------------------------------------------------------------------------
#
# CMakeLists for lnDSO
# An opensource firmware for the DSO SHELL 150
# With optional hardware mods :
#   - Alternative rotary encoder wiring : MUCH Better, almost mandatory
#
#-----------------------------------------------------------------------------
PROJECT(pb_test NONE) # the NONE is very important !
cmake_minimum_required(VERSION 3.0)

enable_language(C CXX ) # this is important too!

MESSAGE(STATUS "===============")
MESSAGE(STATUS "Starting DSO_pb")
MESSAGE(STATUS "===============")
#____________________
# Options
#____________________
SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)
#
ADD_DEFINITIONS("-g3")
#
SET(PB ${CMAKE_CURRENT_SOURCE_DIR}/../tmp)
include_directories( ${PB} )
ADD_LIBRARY(gen_pb STATIC ${PB}/messaging.pb.cpp)
#
ADD_EXECUTABLE(cpp main.cpp)
TARGET_LINK_LIBRARIES(cpp gen_pb)


MESSAGE(STATUS "\tFinal name is ${EXTENSION}")
