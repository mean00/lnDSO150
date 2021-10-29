#include "lnArduino.h"
#include "gd32_8bits.h"
#include "lnStopWatch.h"
#include "assets/gfx/dso150nb_compressed.h"
#include "dso_gfx.h"
#include "dso_menuEngine.h"
#include "dso_display.h"
#include "dso_control.h"
#include "dso_adc_capture.h"

xFastEventGroup            *evtGroup;
extern DSOControl          *control;
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
    redrawEverything();
    
    Logger("Setting 2v max gain\n");
    DSOCapture::setVoltageRange(DSOCapture::DSO_VOLTAGE_1V);    
    DSOCapture::setTimeBase(DSOCapture::DSO_TIME_BASE_1MS);    
    
    DSOInputGain::readCalibrationValue();
    
    evtGroup=new xFastEventGroup;
    evtGroup->takeOwnership();
    control->changeCb(ControlCb);
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
            // display
            // next
            int fq=DSOCapture::computeFrequency();
            DSOCapture::getData(nb,captureBuffer);
            // convert data to display
            float displayGain=DSOCapture::getVoltToPix();
            for(int i=0;i<nb;i++)
            {
                float f=captureBuffer[i];
                if(f>vMax) vMax=f;
                if(f<vMin) vMin=f;
                f*=displayGain;
                int d=100-(int)(f+0.5);
                if(d>199) d=199;
                if(d<0) d=0;
                displayData[i]=d;
            }
            
            
            // we can ask for the next one now
            DSOCapture::startCapture(240);
            DSODisplay::drawWaveForm(nb,displayData);
            DSODisplay::drawMinMax(vMin,vMax);
            DSODisplay::drawFq(fq);
            
        }
        if(evt & DSO_EVT_COUPLING)
        {
            DSODisplay::drawCoupling(control->geCouplingStateAsText(),false);
        }
        
    }
}
// EOF
