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
cmake_policy(SET CMP0079 NEW)
#____________________
# Options
#____________________
SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)


OPTION(USE_ROTARY_FULL_STEP     "Use full step for rotary encoder, else half step"  FALSE)
OPTION(USE_USB                  "Enable USB support"                                FALSE)
OPTION(USE_SMALLFOOTPRINT       "Build for 64k flash targets"                       FALSE)
OPTION(USE_FNIRSI_BUTTON        "Use FNIRSI button layout"                          FALSE)

IF(USE_NO_DEFAULT)

ELSE(USE_NO_DEFAULT)
  include(./build_default.cmake)
ENDIF(USE_NO_DEFAULT)


SET(LN_USB_NB_CDC  1 CACHE INTERNAL "")
SET(LN_USB_NB_HID  0 CACHE INTERNAL "")
SET(LN_USB_DFU_RUNTIME 1 CACHE INTERNAL "")


SET(LN_ENABLE_I2C   FALSE CACHE INTERNAL "")
SET(LN_ENABLE_SPI   FALSE CACHE INTERNAL "")
SET(LN_ENABLE_UART  FALSE CACHE INTERNAL "")
SET(LN_ENABLE_RUST  FALSE CACHE INTERNAL "")


IF(LN_ENABLE_USBD)
    ADD_DEFINITIONS("-DSTM32F103xB")
    SET(LN_ENABLE_USBD  True CACHE INTERNAL "")
ENDIF()


SET(AF_FOLDER  ${CMAKE_SOURCE_DIR}/lnArduino/)
include(./mcuSelect.cmake)

SET(CMAKE_TOOLCHAIN_FILE ${AF_FOLDER}/lnArduino.cmake)
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${AF_FOLDER}/cmake CACHE INTERNAL "")

include(./FindGitSvn.cmake)
SET(CMAKE_TOOLCHAIN_FILE ${AF_FOLDER}/lnArduino.cmake)

PROJECT(lnDSO NONE) # the NONE is very important !

enable_language(C CXX ASM) # this is important too!

#

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/include)

SET(LN_INITIAL_STACK_SIZE 1600 CACHE INTERNAL "")
SET(LN_INITIAL_TASK_PRIORITY 4 CACHE INTERNAL "")

add_subdirectory(${AF_FOLDER})

include(${AF_FOLDER}/setup.cmake)

admGetGitRevision(${CMAKE_SOURCE_DIR} git_rev)

SET(EXTENSION "_${LN_MCU}")

IF(USE_VANILLA_HW)
    SET(EXTENSION "${EXTENSION}_vanilla")
ELSE(USE_VANILLA_HW)
    ADD_DEFINITIONS("-DUSE_RXTX_PIN_FOR_ROTARY")
    SET(EXTENSION "${EXTENSION}_mod")
ENDIF(USE_VANILLA_HW)

IF(USE_FNIRSI_BUTTON)
    SET(EXTENSION "${EXTENSION}_fnirsi") 
    SET(USE_ROTARY_FULL_STEP True CACHE INTERNAL "")
ENDIF()

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
IF(USE_SMALLFOOTPRINT)
    SET(EXTENSION "${EXTENSION}_small") 
ELSE()
  SET(DSO_EXTRA_LIB simplerILI9341HS)
  SET(SIMPLER_ILI9341_FONT_COMPRESSION ON CACHE INTERNAL "")
  SET(SIMPLER_ILI9341_2BPP_SUPPORT     ON CACHE INTERNAL "")
ENDIF()
include_directories(externals/simplerILI9341/cpp/platformExample/lnArduino)
SET(ILI9341_DRIVER  platformExample/lnArduino/ili_ln8bits.cpp)
ADD_SUBDIRECTORY(externals/simplerILI9341/cpp)



include_directories(externals/simplerILI9341/cpp)
#target_link_libraries(simplerILI9341  PRIVATE lnArduino)
#
include_directories(capture)
USE_LIBRARY(NeVerMind)

ADD_SUBDIRECTORY(assets)
ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(capture)
#ADD_SUBDIRECTORY(testFunc)
#
IF(LN_ENABLE_USBD)    
    USE_LIBRARY(tinyUsb)
    SET(EXTENSION "${EXTENSION}_USB")
ENDIF(LN_ENABLE_USBD)

IF(LN_ENABLE_USBD)
    SET(NPB ${CMAKE_SOURCE_DIR}/externals/nanopb)    
    include_directories( ${NPB} )    
    #ADD_LIBRARY(nanopb ${NPB}/pb_decode.c ${NPB}/pb_encode.c )
    ADD_SUBDIRECTORY( ${NPB} )
ENDIF(LN_ENABLE_USBD)

IF(USE_CLANG)
    SET(EXTENSION "${EXTENSION}_CLANG")
ELSE()    
    SET(EXTENSION "${EXTENSION}_GCC")
ENDIF()

#____________________
# Build
#____________________
GENERATE_GD32_FIRMWARE(lnDSO)

IF(LN_ENABLE_RUST)
  TARGET_LINK_LIBRARIES( lnDSO  lnarduino_c_bindings)
  TARGET_LINK_LIBRARIES( lnDSO  rsdso)
ENDIF()
TARGET_LINK_LIBRARIES( lnDSO  captureLib )
TARGET_LINK_LIBRARIES( lnDSO  sourceLib )
IF(LN_ENABLE_USBD)
    TARGET_LINK_LIBRARIES( lnDSO  nanopb)
ENDIF()
TARGET_LINK_LIBRARIES( lnDSO  simplerILI9341 ${DSO_EXTRA_LIB} assets ${FP_LIB} )

HASH_GD32_FIRMWARE(lnDSO)

# Summary
MESSAGE(STATUS "Configuration:")
MESSAGE(STATUS "\tBuild folder ${CMAKE_BINARY_DIR}")
MESSAGE(STATUS "\tUsing ${LN_MCU} based MCU at ${LN_MCU_SPEED} Hz")
MESSAGE(STATUS "\tUsing ${LN_MCU_FLASH_SIZE} kB of flash (small = ${USE_SMALLFOOTPRINT})")
MESSAGE(STATUS "\tUSB control enabled : ${LN_ENABLE_USBD}")
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

set_target_properties(lnDSO PROPERTIES OUTPUT_NAME lnDSO${EXTENSION})

MESSAGE(STATUS "\tFinal name is ${EXTENSION}")
