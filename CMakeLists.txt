#-----------------------------------------------------------------------------
#
# CMakeLists for lnDSO
# An opensource firmware for the DSO SHELL 150
# With optional hardware mods :
#   - Alternative rotary encoder wiring : MUCH Better, almost mandatory
#
#-----------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.0)
MESSAGE(STATUS "===============")
MESSAGE(STATUS "Starting lnDSO")
MESSAGE(STATUS "===============")

SET(AF_FOLDER  ${CMAKE_SOURCE_DIR}/lnArduino/)
include(./mcuSelect.cmake)
SET(CMAKE_TOOLCHAIN_FILE ${AF_FOLDER}/lnArduino.cmake)

PROJECT(lnDSO NONE) # the NONE is very important !

enable_language(C CXX ASM) # this is important too!

#____________________
# Options
#____________________
OPTION(USE_VANILLA_HW "user unmodified pins for rotary encoder"             FALSE)
OPTION(USE_ROTARY_FULL_STEP "Use full step for rotary encoder, else half step"  FALSE)
OPTION(USE_VANILLA_HW   "Use unmodified rotary encoder"                     FALSE)
OPTION(USE_USB          "Enable USB support"                                FALSE)

#

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/include)
add_subdirectory(${AF_FOLDER})

include(${AF_FOLDER}/setup.cmake)


SET(EXTENSION "_${LN_MCU}")

IF(USE_VANILLA_HW)    
    SET(EXTENSION "${EXTENSION}_vanilla")
ELSE(USE_VANILLA_HW)    
    ADD_DEFINITIONS("-DUSE_RXTX_PIN_FOR_ROTARY")
    SET(EXTENSION "${EXTENSION}_mod")
ENDIF(USE_VANILLA_HW)

IF(USE_ROTARY_FULL_STEP)    
    SET(EXTENSION "${EXTENSION}_fstep")
ELSE(USE_ROTARY_FULL_STEP)    
    ADD_DEFINITIONS("-DDSO_ROTARY_FULL_STEP")
    SET(EXTENSION "${EXTENSION}_hstep")
ENDIF(USE_ROTARY_FULL_STEP)


math(EXPR MCU_SPEED_M "${LN_MCU_SPEED}/1000000")
SET(EXTENSION "${EXTENSION}_${MCU_SPEED_M}M")

 
include_directories(src)

#_______________________
# FPU
#_______________________
IF("${LN_ARCH}" STREQUAL "RISCV")
    SET(FP_LIB rvfp)
ELSEIF("${LN_ARCH}" STREQUAL "ARM")
    IF("${LN_MCU}" STREQUAL "M3")
        SET(FP_LIB qfpm3)
    ENDIF("${LN_MCU}" STREQUAL "M3")
ENDIF("${LN_ARCH}" STREQUAL "RISCV")
ADD_SUBDIRECTORY(fp)
include_directories(fp)
# FreeRTOS
SET(USE_TRIGGER True)
#_______________________
#  Display
#_______________________
SET(SIMPLER_ILI9341_FONT_COMPRESSION ON CACHE INTERNAL "")
include_directories(externals/simplerILI9341/platformExample/lnArduino)
SET(ILI9341_DRIVER  platformExample/lnArduino/ili_ln8bits.cpp)
ADD_SUBDIRECTORY(externals/simplerILI9341) 
include_directories(externals/simplerILI9341)
#
include_directories(capture)
USE_LIBRARY(NeVerMind)

ADD_SUBDIRECTORY(assets) 
ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(capture)
ADD_SUBDIRECTORY(testFunc)
#
#____________________
# Build
#____________________
GENERATE_GD32_FIRMWARE(lnDSO${EXTENSION})
TARGET_LINK_LIBRARIES( lnDSO${EXTENSION}  captureLib )
TARGET_LINK_LIBRARIES( lnDSO${EXTENSION}  sourceLib testLib)
TARGET_LINK_LIBRARIES( lnDSO${EXTENSION}  simplerILI9341 simplerILI9341HS assets ${FP_LIB})

# Summary
MESSAGE(STATUS "Configuration:")
MESSAGE(STATUS "\tBuild folder ${CMAKE_BINARY_DIR}")
MESSAGE(STATUS "\tUsing ${LN_MCU} based MCU")
if(USE_VANILLA_HW)
    MESSAGE(STATUS "\tUsing vanilla HW")
else(USE_VANILLA_HW)
    MESSAGE(STATUS "\tUsing rotary encoder modification (PB14/PB15) ")
endif(USE_VANILLA_HW)
IF(USE_ROTARY_FULL_STEP)
     MESSAGE(STATUS "\tUsing rotary encoder Full step ")
ELSE()
     MESSAGE(STATUS "\tUsing rotary encoder Half step ")
ENDIF()

#
MESSAGE(STATUS "\tUsing ${LN_MCU} MCU at ${LN_MCU_SPEED} Hz")
MESSAGE(STATUS "\tFinal name is ${EXTENSION}")
