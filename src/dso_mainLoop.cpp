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
extern void processUiEvent();
extern void initUiEvent();

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
void mainLoop()
{
    DSODisplay::cleanup();
    DSODisplay::drawGrid();
    DSODisplay::drawStatsBackGround();

    evtGroup=new xFastEventGroup;
    control->changeCb(ControlCb);
    initUiEvent();
    while(1)
    {
        int evt=evtGroup->waitEvents(0xff,10000);
        if(evt & DSO_EVT_UI)
        {
            processUiEvent();
        }
    }
}
// EOF
