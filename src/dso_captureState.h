#pragma once
#include "lnArduino.h"
#include "dso_display.h"
#include "dso_control.h"
#include "dso_capture.h"

/**
 * 
 */
class DSOCaptureState
{
public:
    static void startCapture();
    static void stopCapture();
    static void toggleState();    
    static void captureProcessed();
    static void userPress();
    static DSO_ArmingMode  getArmingMode();
    static void setArmingMode(DSO_ArmingMode mode);
};