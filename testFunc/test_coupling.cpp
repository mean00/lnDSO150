
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
        const char *t=control->geCouplingStateAsText();
        DSO_GFX::printxy(1,5,t);
        Logger("Coupling %s\n",t);
        xDelay(200);
    }
    
    
}