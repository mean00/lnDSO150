#-----------------------------------------------------------------------------
#
# CMAkeLists for DSO-STM32
# An opensource firmware for the DSO SHELL 150
# With optional hardware mods :
#   - Alterntive rotary encoder wiring : MUCH Better, almost mandatory
#   - Use GD32F303 as another MCU (much faster, better ADC, more memory)
#
#-----------------------------------------------------------------------------


OPTION(USE_VANILLA_HW "Use vanilla DSO HW" FALSE)  # Use PB14/PB15 for rotary encoder
OPTION(USE_GD32F303   "Use GD303 MCU" TRUE)        # Else it is STM32F103
# Select platform and installation for extra arduino files AND toolchain (compiler...)
SET(PLATFORM_ARCHITECTURE "STM32F1")
# Do we have patch around ?
include(cmake_utils/FindPatch.cmake)
#
# Patch Arduino_stm32 if needed to add gd32f303 support
#
include(./platformConfig.cmake)
IF(NOT EXISTS "${PLATFORM_PATH}/patched")
    MESSAGE(STATUS "Patching file in ${PLATFORM_PATH} to add gd32f303 support")
    patch_file_p(2 "${PLATFORM_PATH}" "${CMAKE_SOURCE_DIR}/patches/gd32_patch.diff")
    file(WRITE "${PLATFORM_PATH}/patched" "patched")
ENDIF(NOT EXISTS "${PLATFORM_PATH}/patched")


cmake_minimum_required(VERSION 2.8)
# Must be named arduino
#  else the cmake code will look for ARDUINO
# while the name will be the name of the folder
# and here we go
IF(USE_GD32F303)
    SET(MCU_SPEED             96000000)                 # Max speed, above 96M USB will not work (?)
                                                        # 96 gives us exact sampling frequencies, 120 does not
ENDIF(USE_GD32F303)

#
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
SET(CMAKE_TOOLCHAIN_FILE cmake/ArduinoToolchain.cmake)
Project("dso_stm32" C CXX ASM)

# Board setup
IF(USE_GD32F303)
    SET(EXTENSION             "_GD32F303")
    set(ARDUINO_DEFAULT_BOARD genericGD32F303C)        # Default Board ID, when not specified
    set(ARDUINO_CPU           GD32F303C8)
    ADD_DEFINITIONS("-DHIGH_SPEED_ADC")
ELSE(USE_GD32F303)
    SET(EXTENSION             "_STM32F103")
    set(ARDUINO_DEFAULT_BOARD genericSTM32F103C)        # Default Board ID, when not specified
    set(ARDUINO_CPU           STM32F103C8)
ENDIF(USE_GD32F303)



# Common stuff, we allow update through USB/DFU

set(ARDUINO_DEFAULT_PORT ttyACM0) # Default Port, when not specified
SET(ARDUINO_UPLOAD_METHOD DFUUpload)
set(ARDUINO_CPUMENU .menu.cpu.${ARDUINO_CPU})
SET(libPrefix ${ARDUINO_DEFAULT_BOARD}_)


MESSAGE(STATUS "Starting dso-stm32")


IF(USE_VANILLA_HW)    
    SET(EXTENSION "${EXTENSION}_vanilla")
ELSE(USE_VANILLA_HW)    
    ADD_DEFINITIONS("-DUSE_RXTX_PIN_FOR_ROTARY")
    SET(EXTENSION "${EXTENSION}_usb")
ENDIF(USE_VANILLA_HW)
 
 
include_directories(MapleFreeRTOS1000//Source/include)
include_directories(MapleFreeRTOS1000//)
include_directories(Adafruit_GFX)
include_directories(Adafruit_TFTLCD_8bit_STM32)
include_directories(.)
link_directories( ${CUSTOM_LIB_DIR})
include_directories(adc)
include_directories(captureEngine)

# FreeRTOS
SET(PORTNAME Stm32duino)
ADD_SUBDIRECTORY(adc) 
ADD_SUBDIRECTORY(MapleFreeRTOS1000/)
ADD_SUBDIRECTORY(testFunc/)
ADD_SUBDIRECTORY(captureEngine/)
#
generate_arduino_firmware(Dso150${EXTENSION} 
                SKETCH dso150.ino
                SRCS    myCode.cpp  dso_test_signal.cpp dso_control.cpp   dso_eeprom.cpp dso_calibrate.cpp dso_display.cpp   stopWatch.cpp dso_frequency.cpp 
                dso_menu.cpp dso_menuEngine.cpp  dso_mainUI.cpp dso_menuButton.cpp fancyLock.cpp dso_autoSetup.cpp  dso_gfx.cpp dso_adc_gain.cpp cpuID.cpp embedded_printf/printf.c helpers/helper_pwm.cpp
                PORT ${ARDUINO_DEFAULT_PORT}
                BOARD_CPU ${ARDUINO_CPU}
                )
TARGET_LINK_LIBRARIES( Dso150${EXTENSION}  ${libPrefix}FreeRTOS)
TARGET_LINK_LIBRARIES( Dso150${EXTENSION}  ${libPrefix}captureEngine)
TARGET_LINK_LIBRARIES( Dso150${EXTENSION}  ${libPrefix}adc)
TARGET_LINK_LIBRARIES( Dso150${EXTENSION}  ${libPrefix}tests)

# Summary
MESSAGE(STATUS "Configuration:")
if(USE_VANILLA_HW)
    MESSAGE(STATUS "\tUsing vanilla HW")
else(USE_VANILLA_HW)
    MESSAGE(STATUS "\tUsing rotary encoder modification (PB14/PB15) ")
endif(USE_VANILLA_HW)
#
if(USE_GD32F303)
    MESSAGE(STATUS "\tUsing GD32F303 MCU ")
else(USE_GD32F303)
    MESSAGE(STATUS "\tUsing STM32F103 MCU ")
endif(USE_GD32F303)
# EOF
# EOF
