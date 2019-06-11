
# Select platform and installation for extra arduino files AND toolchain (compiler...)
SET(PLATFORM_ARCHITECTURE "STM32F1")
# Must be named arduino
#  else the cmake code will look for ARDUINO
# while the name will be the name of the folder
IF(NOT PLATFORM_PATH)
    SET(PLATFORM_PATH "/home/fx/Arduino/hardware/arduino/")
    MESSAGE(STATUS "Using default value for PLATFORM_PATH ${PLATFORM_PATH}")
ENDIF(NOT PLATFORM_PATH)
IF(NOT PLATFORM_TOOLCHAIN_PATH)
    SET(PLATFORM_TOOLCHAIN_PATH "/home/fx/.arduino15/packages/arduino/tools/arm-none-eabi-gcc/4.8.3-2014q1/bin/")
    MESSAGE(STATUS "Using default value for PLATFORM_TOOLCHAIN_PATH ${PLATFORM_TOOLCHAIN_PATH}")
ENDIF(NOT PLATFORM_TOOLCHAIN_PATH)
IF(NOT CUSTOM_LIB_DIR)
    # Where you store your downloaded libs or manually installed
    SET(CUSTOM_LIB_DIR /home/fx/Arduino/libraries)
    MESSAGE(STATUS "Using default value for CUSTOM_LIB_DIR ${CUSTOM_LIB_DIR}")
ENDIF(NOT CUSTOM_LIB_DIR)


# and here we go
SET(CMAKE_TOOLCHAIN_FILE cmake/ArduinoToolchain.cmake)


cmake_minimum_required(VERSION 2.8)
Project("dso_stm32" C CXX ASM)

MESSAGE(STATUS "Starting dso-stm32")


OPTION(USE_VANILLA_HW "Use vanilla DSO HW" FALSE)
IF(USE_VANILLA_HW)    
    SET(EXTENSION "_vanilla")
ELSE(USE_VANILLA_HW)    
    ADD_DEFINITIONS("-DUSE_RXTX_PIN_FOR_ROTARY")
    SET(EXTENSION "_usb")
ENDIF(USE_VANILLA_HW)
  
  

#print_board_list()
#ADD_DEFINITIONS("-O1")

set(ARDUINO_DEFAULT_BOARD genericSTM32F103C)        # Default Board ID, when not specified
set(ARDUINO_DEFAULT_PORT ttyACM0) # Default Port, when not specified
set(ARDUINO_CPU          STM32F103C8)
SET(ARDUINO_UPLOAD_METHOD DFUUpload)
#SET(ARDUINO_UPLOAD_METHOD BMUpload)

# this is needed for the nano, it should be set automatically, maybe something changed with sdk 1.8.x ?
set(ARDUINO_CPUMENU .menu.cpu.${ARDUINO_CPU})
#PRINT_SETTINGS(${ARDUINO_DEFAULT_BOARD})

SET(MapleFreeRTOS1000_RECURSE True)

link_directories( ${CUSTOM_LIB_DIR})
#
# Be careful with the includes in the ino files
# #include <foo.h> // this is a comment
# will fail
# same with extra tabs or space after the #include "blah.h"
#
SET(libPrefix ${ARDUINO_DEFAULT_BOARD}_)

#SET(ILI ${CMAKE_CURRENT_SOURCE_DIR}/ILI9341_extended/)

#include_directories(${ILI})
#include_directories(myAdc)


#generate_arduino_library( simpler_INA219
                        #SRCS  simpler_INA219/simpler_INA219.cpp
                        #BOARD ${ARDUINO_DEFAULT_BOARD})

#INCLUDE_DIRECTORIES(simpler_INA219)

# This reduces size by 10% or so
#ADD_DEFINITIONS("-ffunction-sections -fdata-sections ")
#SET(MCP23017_FREERTOS ${CMAKE_CURRENT_SOURCE_DIR}/mcp23017/example/FreeRtos)
#include_directories(${MCP23017_FREERTOS})
generate_arduino_firmware(Dso150${EXTENSION} 
                SKETCH dso150.ino
                #SRCS   ${ILI}/ILI9341_exi2cPowerSupplytended.cpp ${ILI}/ILI9341_extended_glyph.cpp myCode.cpp  simplerMCP4725/simplerMCP4725.cpp ${MCP23017_FREERTOS}/MCP23017_rtos.cpp  myAdc/myAdc.cpp
                SRCS    myCode.cpp  dso_test_signal.cpp dso_control.cpp testFunc/test_testSignal.cpp testFunc/test_buttons.cpp testFunc/test_adc.cpp dso_adc_fast.cpp dso_adc_slow.cpp  testFunc/test_adc2.cpp testFunc/test_display.cpp  testFunc/test_calibrate.cpp testFunc/test_i2c.cpp  dso_eeprom.cpp dso_calibrate.cpp dso_display.cpp testFunc/test_adc3.cpp 
                dso_frequency.cpp dso_capture_dma.cpp dso_capture_timer.cpp dso_capture.cpp  dso_capture_modes.cpp dso_capture_const.cpp dso_menu.cpp
                testFunc/test_capture.cpp testFunc/test_trigger.cpp testFunc/test_watchdog.cpp dso_mainUI.cpp
                PORT ${ARDUINO_DEFAULT_PORT}
                BOARD_CPU ${ARDUINO_CPU}
                )

# EOF
