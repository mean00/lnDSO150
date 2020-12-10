#
SET(SRCS 
Adafruit_GFX.cpp  Adafruit_TFTLCD_8bit_STM32.cpp  extended.cpp  extended_glyph.cpp    ili9341.cpp  st7789.cpp
) 
#add_library(${libPrefix}adc STATIC ${SRCS})
generate_arduino_library(${libPrefix}TFT 
                            SRCS ${SRCS}
                            BOARD ${ARDUINO_DEFAULT_BOARD}
                        )
target_include_directories(${libPrefix}TFT PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}  ${CMAKE_CURRENT_SOURCE_DIR}/../src)
