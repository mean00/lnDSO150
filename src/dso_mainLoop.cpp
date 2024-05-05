#include "dso_calibrate.h"
#include "dso_capture.h"
#include "dso_control.h"
#include "dso_display.h"
#include "dso_events.h"
#include "dso_gfx.h"
#include "dso_menuEngine.h"
#include "gd32/nvm_gd32.h"
#include "ili_ln8bits.h"
#include "lnArduino.h"
#include "lnStopWatch.h"
//--
lnFastEventGroup *evtGroup;
extern DSOControl *control;
DSO_ArmingMode armingMode = DSO_CAPTURE_MULTI;

extern void processUiEvent();
extern void initUiEvent();
extern void showCapture();
//--
float *captureBuffer;
uint8_t *displayData;

extern void dsoInitUsb();
extern void showTriggerState();
extern void processUsbEvent();
/**
 *
 * @param evt
 */
void ControlCb(DSOControl::DSOEvent evt)
{
    switch (evt)
    {
    case DSOControl::DSOEventCoupling:
        evtGroup->setEvents(DSO_EVT_COUPLING);
        break;

    case DSOControl::DSOEventControl:
        evtGroup->setEvents(DSO_EVT_UI);
        break;
    default:
        xAssert(0);
        break;
    }
}

/**
 *
 * @param evt
 */
void CaptureCb()
{
    evtGroup->setEvents(DSO_EVT_CAPTURE);
}
/**
 *
 */
void redrawEverything()
{
    DSODisplay::cleanup();
    DSO_GFX::setSmallFont();
    DSODisplay::drawGrid();
    DSODisplay::drawStatsBackGround();
    DSODisplay::drawCoupling(control->geCouplingStateAsText(), false);
    DSODisplay::drawTrigger(DSOCapture::getTriggerModeAsText(), false);
}

/**
 *
 */
extern lnNvm *nvm;
void mainLoop()
{
    captureBuffer = new float[240];
    displayData = new uint8_t[240];

    DSOCapture::setVoltageRange(DSOCapture::DSO_VOLTAGE_1V);
    DSOCapture::setTimeBase(DSOCapture::DSO_TIME_BASE_1MS);

    DSOInputGain::preComputeMultiplier();
    control->loadSettings();

    evtGroup = new lnFastEventGroup;
    evtGroup->takeOwnership();
    control->changeCb(ControlCb);

    Logger("Loading calibration data\n");
    if (!DSOCalibrate::loadCalibrationData())
    {
        nvm->format();
        DSOCalibrate::zeroCalibrate();
    }
    DSOInputGain::postComputeMultiplier();

    DSOCapture::setCouplingMode(control->getCouplingState() == DSOControl::DSO_COUPLING_AC);
    DSOCapture::setTriggerMode(DSOCapture::Trigger_Rising);
    DSOCapture::setTriggerMode(DSOCapture::Trigger_Run);
    DSOCapture::setTriggerVoltage(1.0);

    redrawEverything();
    initUiEvent();
    DSOCapture::setCb(CaptureCb);
    DSOCapture::startCapture(240);

    // Start USB
#ifdef DSO_ENABLE_USB
    dsoInitUsb();
#endif

    // and go!
    int loop = 0;
    while (1)
    {
        xDelay(1); // if we go too fast , it will be stuck (?)
        int evt = evtGroup->waitEvents(0xff, 100);
        if (!evt)
        {
            showTriggerState();
            loop &= 0xff;
            if (!loop)
                Logger("*\n");
            continue;
        }

        if (evt & DSO_EVT_UI)
        {
            processUiEvent();
        }
#ifdef DSO_ENABLE_USB
        if (evt & DSO_EVT_USB)
        {
            processUsbEvent();
        }
#endif
        if (evt & DSO_EVT_CAPTURE)
        {
            showCapture();
        }
        if (evt & DSO_EVT_COUPLING)
        {
            DSODisplay::drawCoupling(control->geCouplingStateAsText(), false);
            DSOCapture::setCouplingMode(control->getCouplingState() == DSOControl::DSO_COUPLING_AC);
        }
        showTriggerState();
    }
}
// EOF
