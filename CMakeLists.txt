
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
    # Use gcc 8.2.1 from MCU eclipse in nano mode
    SET(ARDUINO_USE_NEWLIB 1)
    SET(PLATFORM_TOOLCHAIN_PATH "/home/fx/Arduino_stm32/arm-none-eabi-gcc/download/gcc-arm-none-eabi-8.2.1-1.7/bin")
    #SET(PLATFORM_TOOLCHAIN_PATH "/home/fx/.arduino15/packages/arduino/tools/arm-none-eabi-gcc/4.8.3-2014q1/bin/")
    MESSAGE(STATUS "Using default value for PLATFORM_TOOLCHAIN_PATH ${PLATFORM_TOOLCHAIN_PATH}")
ENDIF(NOT PLATFORM_TOOLCHAIN_PATH)
IF(NOT CUSTOM_LIB_DIR)
    # Where you store your downloaded libs or manually installed
    SET(CUSTOM_LIB_DIR /home/fx/Arduino/libraries)
    MESSAGE(STATUS "Using default value for CUSTOM_LIB_DIR ${CUSTOM_LIB_DIR}")
ENDIF(NOT CUSTOM_LIB_DIR)


# and here we go
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
SET(CMAKE_TOOLCHAIN_FILE cmake/ArduinoToolchain.cmake)


cmake_minimum_required(VERSION 2.8)
Project("dso_stm32" C CXX ASM)
# Board setup
set(ARDUINO_DEFAULT_BOARD genericSTM32F103C)        # Default Board ID, when not specified
set(ARDUINO_DEFAULT_PORT ttyACM0) # Default Port, when not specified
set(ARDUINO_CPU          STM32F103C8)
SET(ARDUINO_UPLOAD_METHOD DFUUpload)
set(ARDUINO_CPUMENU .menu.cpu.${ARDUINO_CPU})
SET(libPrefix ${ARDUINO_DEFAULT_BOARD}_)


MESSAGE(STATUS "Starting dso-stm32")


OPTION(USE_VANILLA_HW "Use vanilla DSO HW" FALSE)
IF(USE_VANILLA_HW)    
    SET(EXTENSION "_vanilla")
ELSE(USE_VANILLA_HW)    
    ADD_DEFINITIONS("-DUSE_RXTX_PIN_FOR_ROTARY")
    SET(EXTENSION "_usb")
ENDIF(USE_VANILLA_HW)
  
include_directories(MapleFreeRTOS1000//Source/include)
include_directories(MapleFreeRTOS1000//)
include_directories(.)
link_directories( ${CUSTOM_LIB_DIR})
  
# FreeRTOS
SET(PORTNAME Stm32duino)
ADD_SUBDIRECTORY(MapleFreeRTOS1000/)
generate_arduino_firmware(Dso150${EXTENSION} 
                SKETCH dso150.ino
                SRCS    myCode.cpp  dso_test_signal.cpp dso_control.cpp testFunc/test_testSignal.cpp testFunc/test_buttons.cpp testFunc/test_adc.cpp dso_adc_fast.cpp dso_adc_slow.cpp  testFunc/test_adc2.cpp testFunc/test_display.cpp  testFunc/test_calibrate.cpp testFunc/test_i2c.cpp  dso_eeprom.cpp dso_calibrate.cpp dso_display.cpp testFunc/test_adc3.cpp  stopWatch.cpp dso_frequency.cpp dso_capture_dma.cpp dso_capture_timer.cpp dso_capture.cpp  dso_capture_modes.cpp dso_capture_const.cpp dso_menu.cpp dso_menuEngine.cpp testFunc/test_capture.cpp testFunc/test_trigger.cpp testFunc/test_watchdog.cpp dso_mainUI.cpp dso_menuButton.cpp fancyLock.cpp dso_autoSetup.cpp
                PORT ${ARDUINO_DEFAULT_PORT}
                BOARD_CPU ${ARDUINO_CPU}
                )
TARGET_LINK_LIBRARIES( Dso150${EXTENSION}  ${libPrefix}FreeRTOS)


# EOF
