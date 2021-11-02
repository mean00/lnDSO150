#include "lnArduino.h"
#include "dso_display.h"
#include "dso_control.h"
#include "dso_capture.h"

extern float      *captureBuffer;
extern uint8_t    *displayData;
uint32_t lastRefresh=0;

/**
 */
void showCapture()
{
    float vMin=500,vMax=-500;
    int nb;
    // display
    // next
    uint32_t now=lnGetMs();
    if(now<lastRefresh)
    {
        now=0;
    }
    if(now>lastRefresh+10)
    {
        lastRefresh=now;
        int fq=DSOCapture::computeFrequency();
      //  Logger("F:%d\n",fq);
        DSODisplay::drawFq(fq); 
    }
    
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
           
}
// EOF