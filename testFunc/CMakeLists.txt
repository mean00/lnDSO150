include_directories(../src/ui)
ADD_LIBRARY(  testLib
                testFunc.cpp testControl.cpp testMenu.cpp
                test_testSignal.cpp
                test_drawGfx.cpp
                test_coupling.cpp
                )
