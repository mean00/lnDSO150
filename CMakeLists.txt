#-----------------------------------------------------------------------------
#
# CMAkeLists for DSO-STM32
# An opensource firmware for the DSO SHELL 150
# With optional hardware mods :
#   - Alterntive rotary encoder wiring : MUCH Better, almost mandatory
#   - Use GD32F303 as another MCU (much faster, better ADC, more memory)
#
#-----------------------------------------------------------------------------
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include(select_mcu_stm32)
include(applyPatch)

OPTION(FULL_ROTARY_STEP "Use full step for rotary encoder" FALSE)

#
# Patch Arduino_stm32 if needed to add gd32f303 support
#
include(./platformConfig.cmake)
APPLY_PATCH_IF_NEEDED(patched gd32_patch.diff              "add gd32f303 support") 
APPLY_PATCH_IF_NEEDED(patchedm4 m4_activate_fpu.diff       "activate m4 fpu (gd32f303)") 


cmake_minimum_required(VERSION 2.8)
#
SET(CMAKE_TOOLCHAIN_FILE cmake/ArduinoToolchain.cmake)
Project("dso_stm32" C CXX ASM)

set(ARDUINO_DEFAULT_BOARD ${X_ARDUINO_DEFAULT_BOARD})        # Default Board ID, when not specified
set(ARDUINO_CPU           ${X_ARDUINO_CPU})
SET(ARDUINO_UPLOAD_METHOD ${X_ARDUINO_UPLOAD_METHOD}) # Use blackmagic link, if you comment it out you'll use DFU => 8kB flash
set(ARDUINO_DEFAULT_PORT ttyACM0) # Default Port, when not specified


SET(libPrefix ${ARDUINO_DEFAULT_BOARD}_)


MESSAGE(STATUS "Starting dso-stm32")


IF(USE_VANILLA_HW)    
    SET(EXTENSION "${EXTENSION}_vanilla")
ELSE(USE_VANILLA_HW)    
    ADD_DEFINITIONS("-DUSE_RXTX_PIN_FOR_ROTARY")
    SET(EXTENSION "${EXTENSION}_usb")
ENDIF(USE_VANILLA_HW)

math(EXPR MCU_SPEED_M "${MCU_SPEED}/1000000")
SET(EXTENSION "${EXTENSION}_${MCU_SPEED_M}M")

 
 
include_directories(MapleFreeRTOS1000//Source/include)
include_directories(MapleFreeRTOS1000//)
include_directories(Adafruit_TFTLCD_8bit_STM32)
include_directories(.)
include_directories(adc)
include_directories(captureEngine)
include_directories(src)
include_directories(qfp)

# FreeRTOS
SET(PORTNAME Stm32duino)
SET(USE_TRIGGER True)
# Other modules
ADD_SUBDIRECTORY(adc) 
ADD_SUBDIRECTORY(MapleFreeRTOS1000/)
ADD_SUBDIRECTORY(testFunc/)
ADD_SUBDIRECTORY(captureEngine/)
ADD_SUBDIRECTORY(qfp)
ADD_SUBDIRECTORY(Adafruit_TFTLCD_8bit_STM32)
#
IF(USE_USB)
    ADD_DEFINITIONS(-DUSE_USB)
    SET(USB_SRCS Arduino_STM32/STM32F1/libraries/USBComposite/USBComposite.cpp Arduino_STM32/STM32F1/libraries/USBComposite/USBCompositeSerial.cpp)
    include_directories(Arduino_STM32/STM32F1/libraries/USBComposite)
ELSE(USE_USB)
    SET(USB_SRCS usb_stubs.cpp)
ENDIF(USE_USB)
ADD_SUBDIRECTORY(src)
#
generate_arduino_firmware(Dso150${EXTENSION} 
                SKETCH dso150.ino
                SRCS    myCode.cpp     stopWatch.cpp  cpuID.cpp embedded_printf/printf.c helpers/helper_pwm.cpp  i2c_stubs.c ${USB_SRCS} src/dso_memory.cpp
                PORT ${ARDUINO_DEFAULT_PORT}
                BOARD_CPU ${ARDUINO_CPU}
                )
TARGET_LINK_LIBRARIES( Dso150${EXTENSION}  ${libPrefix}FreeRTOS)
TARGET_LINK_LIBRARIES( Dso150${EXTENSION}  ${libPrefix}captureEngine)
TARGET_LINK_LIBRARIES( Dso150${EXTENSION}  ${libPrefix}adc)
TARGET_LINK_LIBRARIES( Dso150${EXTENSION}  ${libPrefix}TFT)
TARGET_LINK_LIBRARIES( Dso150${EXTENSION}  ${libPrefix}src)
TARGET_LINK_LIBRARIES( Dso150${EXTENSION}  ${libPrefix}qfpm3)
#TARGET_LINK_LIBRARIES( Dso150${EXTENSION}  ${libPrefix}tests)


# Summary
MESSAGE(STATUS "Configuration:")
if(USE_VANILLA_HW)
    MESSAGE(STATUS "\tUsing vanilla HW")
else(USE_VANILLA_HW)
    MESSAGE(STATUS "\tUsing rotary encoder modification (PB14/PB15) ")
endif(USE_VANILLA_HW)
#
MESSAGE(STATUS "\tUsing ${EXTENSION} MCU at ${MCU_SPEED} Hz")
