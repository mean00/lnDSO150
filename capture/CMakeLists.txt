ADD_LIBRARY(captureLib STATIC dso_capture_const.cpp dso_adc_gain.cpp dso_capture.cpp dso_adc.cpp dso_frequency.cpp

)
target_link_libraries(captureLib lnArduino)
