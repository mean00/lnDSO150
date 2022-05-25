#include "lnArduino.h"
#include "dso_display.h"
#include "dso_control.h"
#include "dso_capture.h"
#include "dso_captureState.h"
#include "lnDso_fp.h"
extern float      *captureBuffer;
extern uint8_t    *displayData;
uint32_t lastRefresh=0;
static float voltageOffset=0.0;
extern uint32_t lnGetCycle32();
void  captured();
void  showTriggerState();

/**
 * 
 */
float getVoltageOffset()
{
    return voltageOffset;
}
/**
 */
void setVoltageOffset(float v)
{
    voltageOffset=v;
}
/**
 */
void showCapture()
{
    float vMin=500,vMax=-500;    
    int nb;
    // display
    // next
#warning this is not completely correct    
#if 1   
    uint32_t now=lnGetMs();
    if(now<lastRefresh)
    {
        now=0;
    }    
    if(now>lastRefresh+100) // refresh 10 times a sec
    {
        lastRefresh=now;
        int fq=DSOCapture::computeFrequency();
      //  Logger("F:%d\n",fq);
        DSODisplay::drawFq(fq); 
    }
#endif    
    DSOCapture::getData(nb,captureBuffer,vMin,vMax);
    // convert data to display
    float displayGain=DSOCapture::getVoltToPix();
    // Nano RISCV@108 Mhz
    // ---------------------  Original       Optim
    //  capture -> volt           11   kCycke  11 kCycle
    //  Volt -> Display           24.5 kCycle  10 kCycle
    //    
    // Cortex M4 @ 72 Mhz
    // -------------------     
    // capture->volt              6k cycle
    // Volt -> display            7k cycle
    //
    //  Cortex M3 @72 Mhz (GD)
    // ---------------------------
    // capture->volt              20k cycle
    // Volt -> display            18k cycle
    //
    //  Cortex M3 @72 Mhz (STM)
    // ---------------------------
    // capture->volt              31k cycle
    // Volt -> display            27k cycle

    
    
    //int before=lnGetCycle32();
    float displayGain2=displayGain*2.;
    float invOffset=voltageOffset;
    for(int i=0;i<nb;i++)
    {
        float f=captureBuffer[i];
        f=LN_FP_ADD(f,invOffset);
        f=LN_FP_MUL(f,displayGain2);        
        int d=LN_TO_INT(f);
        d=200-d;
        if(d>398) d=398;
        if(d<0) d=0;
        displayData[i]=d>>1;
    }
//    int after=lnGetCycle32();Logger("Conv 2 display =%d\n",after-before);

    // we can ask for the next one now
    DSOCaptureState::captureProcessed();
    DSODisplay::drawWaveForm(nb,displayData);
    DSODisplay::drawMinMax(vMin,vMax);

    captured();
           
}

int lastCapture=0;
/**
 */

void  captured()
{
    lastCapture=lnGetMs() & 0xffff;
}
/**
 */
bool lastTriggered=false;
void  showTriggerState()
{
    int now=lnGetMs() & 0xffff;
    if(now<lastCapture)
    {
        now=0xffff+now-lastCapture;
    }else
    {
        now=now-lastCapture;
    }
    bool triggered=false;
    if(now<50)
    {
        triggered=true;
    }else
    {
        triggered=false;
    }
    if(triggered!=lastTriggered)
    {
        DSODisplay::drawTriggered(triggered);
    }
    lastTriggered=triggered;
}


// EOF
