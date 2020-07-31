IF(USE_GD32F303 OR USE_GD32F103)
    SET(UTIL dso_adc_slow_gd32.cpp)
ELSE()
    SET(UTIL dso_adc_slow_stm32.cpp)
ENDIF()

SET(SRCS dso_adc_fast.cpp 
dso_adc_fast_trigger.cpp dso_adc_slow_trigger.cpp 
dso_adc_slow.cpp ${UTIL} 
dso_adc_util.cpp dso_adc_addon.cpp dso_adc_updown.cpp) 
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
generate_arduino_library(${libPrefix}adc 
                            SRCS ${SRCS}
                            BOARD ${ARDUINO_DEFAULT_BOARD}
                        )
