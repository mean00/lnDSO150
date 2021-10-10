/***************************************************
 ****************************************************/

#include "lnArduino.h"
#include "dso_test_signal.h"
#include "dso_control.h"
#include "dso_gfx.h"

extern DSO_testSignal *testSignal;
extern DSOControl *control;
/**
 * 
 * @param a
 */
void testTestSignal( void)
{
    int fq[4]={100,1000,10000,50*1000};
    int fqIndex=0;
    control->changeCb(NULL);
    DSO_GFX::clear(0);
    DSO_GFX::setBigFont(true);
    DSO_GFX::setTextColor(0x1f<<5,0);
    DSO_GFX::printxy(2,2,"Switching to fq=100 Hz");
    testSignal->setFrequency(fq[0]);
    char tmp[60];
    while(1)
    {
        int f=fq[fqIndex];
        
        
        int inc=control->getRotaryValue();
        if(inc)
        {
            fqIndex=(fqIndex+inc+16)%4;
            sprintf(tmp,"Switching to fq=%d\n",f);
            Logger(tmp);
            DSO_GFX::clear(0);
            DSO_GFX::printxy(2,2,tmp);
            testSignal->setFrequency(f);
        }
    }
    
        
}

//-
