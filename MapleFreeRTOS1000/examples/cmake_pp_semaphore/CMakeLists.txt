# Select platform and installation for extra arduino files AND toolchain (compiler...)
SET(PLATFORM_ARCHITECTURE "STM32F1")
# Must be named arduino
#  else the cmake code will look for ARDUINO
# while the name will be the name of the folder
#SET(PLATFORM_PATH "/home/fx/Arduino/hardware/arduino/")
# Plain stm32duino
SET(PLATFORM_PATH "/home/fx/Arduino/hardware/rg/arduino")
#SET(PLATFORM_TOOLCHAIN_PATH "/home/fx/.arduino15/./packages/arduino/tools/arm-none-eabi-gcc/4.8.3-2014q1/bin")
SET(PLATFORM_TOOLCHAIN_PATH "/home/fx/.arduino15/packages/arduino/tools/arm-none-eabi-gcc/4.8.3-2014q1/bin/")
#SET(PLATFORM_TOOLCHAIN_PATH "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.0.0/ARMTools/bin/")
#SET(PLATFORM_TOOLCHAIN_PATH "/opt/arm_gcc_2017/gcc-arm-none-eabi-7-2017-q4-major/bin/")
#"/home/fx/.arduino15/packages/STM32/tools/arm-none-eabi-gcc/6-2017-q2-update/bin")
# and here we go
SET(CMAKE_TOOLCHAIN_FILE cmake/ArduinoToolchain.cmake)
MESSAGE(STATUS "Starting hello-stm32")

cmake_minimum_required(VERSION 2.8)
Project("Hello_stm32" C CXX ASM)

print_board_list()
#ADD_DEFINITIONS("-g3 -O0")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)
set(ARDUINO_DEFAULT_BOARD genericSTM32F103C)        # Default Board ID, when not specified
set(ARDUINO_DEFAULT_PORT ttyACM19) # Default Port, when not specified
set(ARDUINO_CPU          STM32F103C8)
# Possible choice for STM32
#  BMP    : BlackMagic
#  STLink : ST link v2
#  DFUUpload: Arduino 
#
# that changes the way the binary is linked, better to use DFU in all cases !
SET(ARDUINO_UPLOAD_METHOD DFUUpload)



# this is needed for the nano, it should be set automatically, maybe something changed with sdk 1.8.x ?
set(ARDUINO_CPUMENU .menu.cpu.${ARDUINO_CPU})
PRINT_SETTINGS(${ARDUINO_DEFAULT_BOARD})

# Where you store your downloaded libs or manually installed
SET(CUSTOM_LIB_DIR /home/fx/Arduino/libraries)

SET(MapleFreeRTOS1000_RECURSE True)
SET(STM32FreeRTOS_RECURSE True)
link_directories( ${CUSTOM_LIB_DIR})
SET(libPrefix ${ARDUINO_DEFAULT_BOARD}_)

generate_arduino_firmware(freertos 
                SKETCH test.ino
                SRCS myCode.cpp 
                PORT ${ARDUINO_DEFAULT_PORT}
                BOARD_CPU ${ARDUINO_CPU}
                )
# EOF
