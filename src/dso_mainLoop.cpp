#include "lnArduino.h"
#include "gd32_8bits.h"
#include "lnStopWatch.h"
#include "assets/gfx/dso150nb_compressed.h"
#include "dso_gfx.h"
#include "dso_menuEngine.h"
#include "dso_display.h"
#include "dso_control.h"
#include "dso_capture_stub.h"
#include "dso_adc_gain.h"

xFastEventGroup            *evtGroup;
extern DSOControl          *control;
extern demoCapture         *capture;
extern void processUiEvent();
extern void initUiEvent();

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
 float      captureBuffer[240];
 uint8_t    displayData[240];
 extern uint16_t calibrationDC[];
 
void mainLoop()
{
    DSODisplay::cleanup();
    DSODisplay::drawGrid();
    DSODisplay::drawStatsBackGround();
    
    Logger("Setting 2v max gain\n");
    //-> BREAK adc1
    DSOInputGain::setGainRange(DSOInputGain::MAX_VOLTAGE_2V);
    
    evtGroup=new xFastEventGroup;
    
    control->changeCb(ControlCb);
    initUiEvent();
    
    DSODisplay::drawCoupling(control->geCouplingStateAsText(),false);
    
    
    capture->setCb(CaptureCb);
    int nb=240;
    capture->startCapture(240);
    
    while(1)
    {
        int evt=evtGroup->waitEvents(0xff,10000);
        if(evt & DSO_EVT_UI)
        {
            processUiEvent();
        }
        if(evt & DSO_EVT_CAPTURE)
        {
            // display
            // next
            capture->getData(nb,captureBuffer);
            
            int   offset=     DSOInputGain::getOffset(0);
            float mul=        DSOInputGain::getMultiplier();
            for(int i=0;i<nb;i++)
            {
                int d=captureBuffer[i];
                d-=offset;
                float f=(float)d*mul;
                displayData[i]=120+f*40;
            }
            DSODisplay::drawWaveForm(nb,displayData);
            capture->startCapture(240);
//            Logger("*\n");
            
        }
        if(evt & DSO_EVT_COUPLING)
        {
            DSODisplay::drawCoupling(control->geCouplingStateAsText(),false);
        }
    }
}
// EOF
