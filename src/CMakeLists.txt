
include_directories(ui)



IF(LN_ENABLE_USBD)
   include_directories(${CMAKE_SOURCE_DIR}/python_usb_control/pb_c)
   
   include(${AF_FOLDER}/libraries/tinyUsb/tiny.cmake)
   SET(PB_MESSAGES ${CMAKE_SOURCE_DIR}/python_usb_control/pb_c/messaging.pb.cpp)
   SET(USB_SRC dso_usbd.cpp dso_usbd_api.cpp ${PB_MESSAGES})   
   LN_APPEND_FLAGS(dso_mainLoop.cpp -DDSO_ENABLE_USB)
ENDIF(LN_ENABLE_USBD)

IF(USE_FNIRSI_BUTTON)
    ADD_DEFINITIONS("-DUSE_FNIRSI_BUTTON")
ENDIF()
ADD_LIBRARY(  sourceLib STATIC
                main.cpp dso_gfx.cpp dso_test_signal.cpp dso_control.cpp
                dso_settingMenu.cpp  ui/dso_menuEngine.cpp dso_display.cpp
                DSO_portBArbitrer.cpp
                dso_display.cpp dso_mainLoop.cpp dso_uiFunctions.cpp
                dso_calibrate.cpp
                dso_autoSetup.cpp
                dso_show_capture.cpp
                dso_test.cpp
                dso_captureState.cpp
                ${USB_SRC}                
                )
target_link_libraries(sourceLib lnArduino tinyUsb)


IF(LN_ENABLE_USBD)
    target_include_directories(sourceLib PRIVATE ${CMAKE_SOURCE_DIR}/externals/nanopb/)
ENDIF(LN_ENABLE_USBD)

TARGET_COMPILE_DEFINITIONS(sourceLib PRIVATE "-DLN_GIT_REV=\"${git_rev}\"")
                
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

target_link_libraries( sourceLib lnArduino )