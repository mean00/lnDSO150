
include_directories(ui)

IF(LN_ENABLE_USBD)
   include(${AF_FOLDER}/libraries/tinyUsb/tiny.cmake)
   SET(USB_SRC dso_usbd.cpp)
   LN_APPEND_FLAGS(dso_mainLoop.cpp -DDSO_ENABLE_USB)
ENDIF(LN_ENABLE_USBD)

ADD_LIBRARY(  sourceLib
                main.cpp dso_gfx.cpp dso_test_signal.cpp dso_control.cpp
                dso_settingMenu.cpp  ui/dso_menuEngine.cpp dso_display.cpp
                DSO_portBArbitrer.cpp
                dso_display.cpp dso_mainLoop.cpp dso_uiFunctions.cpp
                dso_calibrate.cpp
                dso_autoSetup.cpp
                dso_show_capture.cpp
                ${USB_SRC}
                )


IF(NOT USE_VANILLA_HW)
    TARGET_COMPILE_DEFINITIONS(sourceLib PRIVATE "-DUSE_RXTX_PIN_FOR_ROTARY")
ENDIF( )

IF(NOT USE_ROTARY_FULL_STEP)
    TARGET_COMPILE_DEFINITIONS(sourceLib PRIVATE "-DDSO_ROTARY_FULL_STEP")
ENDIF( )


IF("${LN_ARCH}" STREQUAL "RISCV")
    MESSAGE(STATUS "ILI9341 : Using PB8 instead of PB3 for LCD")
    TARGET_COMPILE_OPTIONS(sourceLib PRIVATE "-DUSE_PB8_INSTEAD_OF_PB3")
ENDIF()
