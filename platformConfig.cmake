SET(ARDUINO_USE_NEWLIB 1)
SET(PLATFORM_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Arduino_STM32/STM32F1/")

# Warning the paths are mingw i.e. c:\dev becomes /c/dev
IF(WIN32)
    SET(ARDUINO_SDK_PATH "/c/dev/Arduino")
    SET(PLATFORM_TOOLCHAIN_PATH  "/c/dev/arm83/bin")
ELSE(WIN32)
    # ARDUINO_SDK_PATH should be autodetected
    #SET(PLATFORM_TOOLCHAIN_PATH "/home/fx/Arduino_stm32/arm-none-eabi-gcc/download/gcc-arm-none-eabi-8.2.1-1.7/bin")
    #SET(PLATFORM_TOOLCHAIN_PATH "/home/fx/Arduino_stm32/arm_gcc_9.3/bin")
    #SET(PLATFORM_TOOLCHAIN_PATH "/usr/bin")
    SET(PLATFORM_TOOLCHAIN_PATH "/home/fx/Arduino_stm32/arm-gcc-2020q2/bin")
ENDIF(WIN32)
