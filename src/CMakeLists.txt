ADD_LIBRARY(  sourceLib
                main.cpp dso_gfx.cpp dso_test_signal.cpp dso_control.cpp
                dso_settingMenu.cpp  ui/dso_menuEngine.cpp dso_display.cpp
                DSO_portBArbitrer.cpp 
                )

        
IF(NOT USE_VANILLA_HW)    
    TARGET_COMPILE_DEFINITIONS(sourceLib PRIVATE "-DUSE_RXTX_PIN_FOR_ROTARY")
ENDIF( )

IF(NOT USE_ROTARY_FULL_STEP)    
    TARGET_COMPILE_DEFINITIONS(sourceLib PRIVATE "-DDSO_ROTARY_FULL_STEP")
ENDIF( )
