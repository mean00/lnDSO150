#include "lnArduino.h"
#include "gd32_8bits.h"
#include "lnStopWatch.h"
#include "assets/gfx/dso150nb_compressed.h"
#include "dso_gfx.h"
#include "dso_menuEngine.h"
#include "dso_display.h"
#include "dso_control.h"

xFastEventGroup *evtGroup;
extern DSOControl          *control;

#define DSO_EVT_UI (1<<0)

/**
 * 
 * @param evt
 */
 void ControlCb(DSOControl::DSOEvent evt)
 {
     evtGroup->setEvents(DSO_EVT_UI);
 }

 /**
  * 
  */
 void processUiEvent()
 {
     while(1)
     {
            int ev=control->getQButtonEvent();
            if(!ev) break;
            int kind=ev>>16;
            int key=ev&0xffff;
            Logger("Got event %d on key %d\n",kind,key);
            
     }
     Logger("Rot=%d\n",control->getRotaryValue());
     
 }
/**
 * 
 */
void mainLoop()
{
    DSODisplay::cleanup();
    DSODisplay::drawGrid();
    DSODisplay::drawStatsBackGround();

    evtGroup=new xFastEventGroup;
    control->changeCb(ControlCb);
    
    while(1)
    {
        int evt=evtGroup->waitEvents(0xff,10000);
        if(evt & DSO_EVT_UI)
        {
            processUiEvent();
        }
    }
    
    
}
