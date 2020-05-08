SET(TEST_SRCS
test_adc2.cpp  test_adc3.cpp  test_adc.cpp  test_buttons.cpp  test_calibrate.cpp  
test_capture.cpp  test_display.cpp  test_i2c.cpp  test_testSignal.cpp  test_trigger.cpp  test_watchdog.cpp
test_dualadc.cpp pigOScope.cpp
)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
generate_arduino_library(${libPrefix}tests 
                            SRCS ${TEST_SRCS}
                            BOARD ${ARDUINO_DEFAULT_BOARD}
                        )
