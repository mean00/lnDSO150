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
 }
 
void mainLoop()
{
    
    captureBuffer=new float[240];
    displayData=new uint8_t[240];    
    
    redrawEverything();
    
    Logger("Setting 2v max gain\n");
    DSOCapture::setVoltageRange(DSOCapture::DSO_VOLTAGE_1V);    
    DSOCapture::setTimeBase(DSOCapture::DSO_TIME_BASE_1MS);    
    
    DSOInputGain::readCalibrationValue();
    
    evtGroup=new xFastEventGroup;
    evtGroup->takeOwnership();
    control->changeCb(ControlCb);
    
    
    Logger("Loading calibration data\n");
    if(!DSOCalibrate::loadCalibrationData())
        DSOCalibrate::zeroCalibrate();
    
    initUiEvent();
    
    DSODisplay::drawCoupling(control->geCouplingStateAsText(),false);
    
    
    DSOCapture::setCb(CaptureCb);
    int nb=240;
    DSOCapture::startCapture(240);
    
    while(1)
    {
        xDelay(1);
        int evt=evtGroup->waitEvents(0xff,2*1000);
        if(!evt)
        {
            Logger("*\n");
            continue;
        }
            
        if(evt & DSO_EVT_UI)
        {
            Logger("UI\n");
            processUiEvent();
            Logger("IU\n");
        }
        float vMin=500,vMax=-500;
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
