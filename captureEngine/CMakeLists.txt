
SET(SRCS 
                dso_capture_dma.cpp dso_capture_timer.cpp dso_capture.cpp  dso_capture_modes.cpp dso_capture_const.cpp 
        )
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
generate_arduino_library(${libPrefix}captureEngine 
                            SRCS ${SRCS}
                            BOARD ${ARDUINO_DEFAULT_BOARD}
                        )
