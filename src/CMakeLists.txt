
include_directories(ui)

add_library(  sourceLib STATIC
                main.cpp dso_gfx.cpp dso_test_signal.cpp dso_control.cpp
                dso_settingMenu.cpp  ui/dso_menuEngine.cpp dso_display.cpp
                DSO_portBArbitrer.cpp
                dso_display.cpp dso_mainLoop.cpp dso_uiFunctions.cpp
                dso_calibrate.cpp
                dso_autoSetup.cpp
                dso_show_capture.cpp
                dso_test.cpp
                dso_captureState.cpp
                )
target_link_libraries(sourceLib PRIVATE esprit_dev)

macro(SET_OPTION cond flag)
  if(${cond})
    target_compile_definitions(sourceLib PRIVATE ${flag})
  endif()
endmacro()

macro(SET_NO_OPTION cond flag)
  if(NOT ${cond})
    target_compile_definitions(sourceLib PRIVATE ${flag})
  endif()
endmacro()


if(LN_ENABLE_USBD)
  include(${ESPRIT_ROOT}/libraries/tinyUsb/tiny.cmake)
  target_include_directories(sourceLib PRIVATE ${CMAKE_SOURCE_DIR}/python_usb_control/pb_c)
  set(PB_MESSAGES ${CMAKE_SOURCE_DIR}/python_usb_control/pb_c/messaging.pb.cpp)
  target_sources(sourceLib PRIVATE dso_usbd.cpp dso_usbd_api.cpp ${PB_MESSAGES})
  target_compile_definitions(sourceLib PRIVATE DSO_ENABLE_USB)
  target_include_directories(sourceLib PRIVATE ${CMAKE_SOURCE_DIR}/externals/nanopb/)
  target_link_libraries(sourceLib PRIVATE tinyUsb)
endif()

target_compile_definitions(sourceLib PRIVATE "-DLN_GIT_REV=\"${git_rev}\"")

SET_OPTION(USE_FNIRSI_BUTTON USE_FNIRSI_BUTTON)
SET_NO_OPTION(USE_VANILLA_HW USE_RXTX_PIN_FOR_ROTARY)
SET_NO_OPTION(USE_ROTARY_FULL_STEP DSO_ROTARY_FULL_STEP)


if("${LN_ARCH}" STREQUAL "RISCV")
  message(STATUS "ILI9341 : Using PB8 instead of PB3 for LCD")
  target_compile_options(sourceLib PRIVATE "-DUSE_PB8_INSTEAD_OF_PB3")
endif()

#target_link_libraries( sourceLib lnArduino )
