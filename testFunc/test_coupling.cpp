
#include "lnArduino.h"
#include "dso_control.h"
#include "dso_gfx.h"
#include "lnStopWatch.h"

extern DSOControl * control;
void testCoupling()
{
    DSO_GFX::clear(0);
    while(1)
    {
        const char *p=control->geCouplingStateAsText();
        DSO_GFX::printxy(3,3,p);
        Logger("%s\n",p);
        xDelay(200);
    }
    
    
}