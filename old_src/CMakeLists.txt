

SET(SRCS 
    dso_adc_gain.cpp
    dso_autoSetup.cpp
    dso_calibrate.cpp
    dso_control.cpp
    dso_display.cpp
    dso_eeprom.cpp
    dso_frequency.cpp
    dso_logger.cpp
    dso_gfx.cpp
    dso_mainUI.cpp
    ui/dso_menuButton.cpp
    ui/dso_menu.cpp
    ui/dso_menuEngine.cpp
    dso_test_signal.cpp
    ) 
IF(USE_USB)
    SET(SRCS ${SRCS} dso_usb.cpp dso_usbCommands.cpp)
ENDIF(USE_USB)

add_library(${libPrefix}src 
                            ${SRCS}
                        )
target_include_directories(${libPrefix}src PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

IF(FULL_ROTARY_STEP)
    TARGET_COMPILE_DEFINITIONS(${libPrefix}src PRIVATE "ROTARY_FULL_STEP")
ENDIF(FULL_ROTARY_STEP)
