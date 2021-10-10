
#include "lnArduino.h"
#include "dso_display.h"
#include "dso_gfx.h"

void testDrawGfx()
{
    DSO_GFX::clear(0);
    DSODisplay::drawGrid();
    uint8_t *data=new uint8_t[512];
    
    for(int i=0;i<512;i++)
    {
        data[i]=(i*4)&0xff;
        if(data[i]>199) data[i]=199;
    }
    for(int i=100;i<110;i++)
        data[i]=100;
    int offset=0;
    while(1)
    {
        uint32_t old=lnGetUs();
        DSODisplay::drawWaveForm(240,data+offset);
        Logger("Draw time : %d us\n",lnGetUs()-old);
        offset=(offset+1)&255;
    }
}