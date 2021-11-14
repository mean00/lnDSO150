#include "lnArduino.h"
#include "gd32_8bits.h"
#include "lnStopWatch.h"
#include "dso_gfx.h"
#include "dso_menuEngine.h"
#include "dso_display.h"
#include "dso_control.h"
#include "dso_capture.h"
#include "dso_calibrate.h"
//--
xFastEventGroup            *evtGroup;
extern DSOControl          *control;
extern void processUiEvent();
extern void initUiEvent();
extern void showCapture();
//--
float      *captureBuffer;
uint8_t    *displayData;
extern uint16_t calibrationDC[];

//--
#define DSO_EVT_UI       (1<<0)
#define DSO_EVT_CAPTURE  (1<<1)
#define DSO_EVT_COUPLING (1<<2)
/**
 * 
 * @param evt
 */
 void ControlCb(DSOControl::DSOEvent evt)
 {
     switch(evt)
     {
         case DSOControl::DSOEventCoupling:evtGroup->setEvents(DSO_EVT_COUPLING);break;
             
         case DSOControl::DSOEventControl: evtGroup->setEvents(DSO_EVT_UI);break;
         default: xAssert(0);break;
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
    DSODisplay::drawCoupling(control->geCouplingStateAsText(),false);
    DSODisplay::drawTrigger(DSOCapture::getTriggerModeAsText(),false);
    
 }
/**
 * 
 */
void mainLoop()
{    
    captureBuffer=new float[240];
    displayData=new uint8_t[240];    
    
    
       
    DSOCapture::setVoltageRange(DSOCapture::DSO_VOLTAGE_1V);    
    DSOCapture::setTimeBase(DSOCapture::DSO_TIME_BASE_1MS);    
    
    DSOInputGain::readCalibrationValue();
    
    evtGroup=new xFastEventGroup;
    evtGroup->takeOwnership();
    control->changeCb(ControlCb);
    
    
    Logger("Loading calibration data\n");
    if(!DSOCalibrate::loadCalibrationData())
        DSOCalibrate::zeroCalibrate();
    
    
    DSOCapture::setCouplingMode(control->getCouplingState()==DSOControl::DSO_COUPLING_AC);
    DSOCapture::setTriggerMode(DSOCapture::Trigger_Run);    
    DSOCapture::setTriggerVoltage(0);
    
    redrawEverything();        
    initUiEvent();
    DSOCapture::setCb(CaptureCb);
    DSOCapture::startCapture(240);
    
    while(1)
    {
        xDelay(1); // if we go too fast , it will be stuck (?)
        int evt=evtGroup->waitEvents(0xff,2*1000);
        if(!evt)
        {
            Logger("*\n");
            continue;
        }
            
        if(evt & DSO_EVT_UI)
        {
            processUiEvent();         
        }
        
        if(evt & DSO_EVT_CAPTURE)
        {
            showCapture();            
        }
        if(evt & DSO_EVT_COUPLING)
        {
            DSODisplay::drawCoupling(control->geCouplingStateAsText(),false);
            DSOCapture::setCouplingMode(control->getCouplingState()==DSOControl::DSO_COUPLING_AC);
        }        
    }
}
// EOF
