#-----------------------------------------------------------------------------
#
# CMakeLists for lnDSO
# An opensource firmware for the DSO SHELL 150
# With optional hardware mods :
#   - Alternative rotary encoder wiring : MUCH Better, almost mandatory
#
#-----------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.0)

SET(AF_FOLDER  ${CMAKE_SOURCE_DIR}/lnArduino/)
include(./mcuSelect.cmake)
SET(CMAKE_TOOLCHAIN_FILE ${AF_FOLDER}/lnArduino.cmake)

PROJECT(lnDSO NONE) # the NONE is very important !

enable_language(C CXX ASM) # this is important too!
include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/include)
add_subdirectory(${AF_FOLDER})

include(${AF_FOLDER}/setup.cmake)

MESSAGE(STATUS "Starting dso-stm32")

OPTION(USE_VANILLA_HW   "Use unmodified rotary encoder" FALSE)
OPTION(USE_USB          "Enable USB support" FALSE)

IF(USE_VANILLA_HW)    
    SET(EXTENSION "${EXTENSION}_vanilla")
ELSE(USE_VANILLA_HW)    
    ADD_DEFINITIONS("-DUSE_RXTX_PIN_FOR_ROTARY")
    SET(EXTENSION "${EXTENSION}_rotary")
ENDIF(USE_VANILLA_HW)

math(EXPR MCU_SPEED_M "${LN_MCU_SPEED}/1000000")
SET(EXTENSION "${EXTENSION}_${MCU_SPEED_M}M")

IF("${LN_ARCH}" STREQUAL "RISCV")
    MESSAGE(STATUS "Using PB8 instead of PB3 for LCD")
    ADD_DEFINITIONS("-DUSE_PB8_INSTEAD_OF_PB3")
ENDIF()
 
include_directories(Adafruit_TFTLCD_8bit_STM32)
include_directories(.)
include_directories(adc)
include_directories(captureEngine)
include_directories(src)
include_directories(qfp)

# FreeRTOS
SET(USE_TRIGGER True)
# Other modules
include_directories(externals/simplerILI9341/platformExample/longanNano)
SET(ILI9341_DRIVER  platformExample/longanNano/gd32_8bits.cpp)
ADD_SUBDIRECTORY(externals/simplerILI9341) 
include_directories(externals/simplerILI9341)
ADD_SUBDIRECTORY(assets) 
#ADD_SUBDIRECTORY(adc) 
#AADD_SUBDIRECTORY(testFunc/)
#AADD_SUBDIRECTORY(captureEngine/)
#AADD_SUBDIRECTORY(qfp)
#AADD_SUBDIRECTORY(Adafruit_TFTLCD_8bit_STM32)
#
IF(USE_USB)
    ADD_DEFINITIONS(-DUSE_USB)
    SET(USB_SRCS Arduino_STM32/STM32F1/libraries/USBComposite/USBComposite.cpp Arduino_STM32/STM32F1/libraries/USBComposite/USBCompositeSerial.cpp)
    include_directories(Arduino_STM32/STM32F1/libraries/USBComposite)
ELSE(USE_USB)
    SET(USB_SRCS usb_stubs.cpp)
ENDIF(USE_USB)
#ADD_SUBDIRECTORY(src)
#
GENERATE_GD32_FIRMWARE(lnDSO${EXTENSION}
                src/main.cpp src/dso_gfx.cpp src/dso_test_signal.cpp src/dso_control.cpp
                src/ui/dso_menu.cpp src/ui/dso_menuButton.cpp src/ui/dso_menuEngine.cpp src/dso_display.cpp
                #myCode.cpp     stopWatch.cpp   helpers/helper_pwm.cpp  i2c_stubs.c ${USB_SRCS} src/dso_memory.cpp
                )
TARGET_LINK_LIBRARIES( lnDSO${EXTENSION}  simplerILI9341 assets)
#TARGET_LINK_LIBRARIES( lnDSO${EXTENSION}  ${libPrefix}adc)
##TARGET_LINK_LIBRARIES( lnDSO${EXTENSION}  ${libPrefix}TFT)
#TARGET_LINK_LIBRARIES( lnDSO${EXTENSION}  ${libPrefix}src)
#TARGET_LINK_LIBRARIES( lnDSO${EXTENSION}  ${libPrefix}qfpm3)
#TARGET_LINK_LIBRARIES( lnDSO${EXTENSION}  ${libPrefix}tests)


# Summary
MESSAGE(STATUS "Configuration:")
MESSAGE(STATUS "\tUsing ${LN_MCU} based MCU")
if(USE_VANILLA_HW)
    MESSAGE(STATUS "\tUsing vanilla HW")
else(USE_VANILLA_HW)
    MESSAGE(STATUS "\tUsing rotary encoder modification (PB14/PB15) ")
endif(USE_VANILLA_HW)
    

#
MESSAGE(STATUS "\tUsing ${EXTENSION} MCU at ${LN_MCU_SPEED} Hz")
