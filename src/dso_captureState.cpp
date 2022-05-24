
#include "dso_captureState.h"

static DSO_ArmingMode              armingMode=DSO_CAPTURE_MULTI;
static bool captureStateCapturing=true;

 /**
  *
  */
void DSOCaptureState::toggleState()
{
    if(captureStateCapturing)     
        stopCapture();
    else
        startCapture();
}
/**
 * 
 * 
 */
void DSOCaptureState::stopCapture()
{
    captureStateCapturing=false;
    switch(DSOCapture::state())
    {
        case DSOCapture:: CAPTURE_RUNNING:
        case DSOCapture:: CAPTURE_DONE:
            DSOCapture::stopCapture();
            break;
        default:break;
    }
}

/**
 * 
 */
void DSOCaptureState::startCapture()
{
    captureStateCapturing=true;
    switch(DSOCapture::state())
    {
        case DSOCapture:: CAPTURE_STOPPED:
            DSOCapture::startCapture(240);
            break;
        default:
            break;
    }
}
/**
 * 
 */
void DSOCaptureState::captureProcessed()
{
    switch(armingMode)
    {
        case DSO_CAPTURE_MULTI: 
            DSOCapture::startCapture(240);
            break;
        case DSO_CAPTURE_SINGLE:
            DSOCapture::stopCapture();
            break;
        default: xAssert(0);
            break;
    }
}
/**
 */
DSO_ArmingMode  DSOCaptureState::getArmingMode()
{
    return armingMode;
}
/**
 */
void DSOCaptureState::setArmingMode(DSO_ArmingMode mode)
{
    armingMode=mode;
}
/**
 * 
 */
void DSOCaptureState::userPress()
{
    switch(armingMode)
    {
        case DSO_CAPTURE_MULTI: 
            toggleState();
            break;
        case DSO_CAPTURE_SINGLE:
            startCapture();
            break;
        default: xAssert(0);
            break;
    }
}
  // -- EOF --