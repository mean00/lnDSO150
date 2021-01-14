/***************************************************
 STM32 duino based firmware for DSO SHELL/150
 *  * GPL v2
 * (c) mean 2019 fixounet@free.fr
 ****************************************************/
#include "dso_global.h"
#include "dso_adc.h"
#include "dso_capture.h"
#include "dso_capture_priv.h"

#include "DSO_config.h"
#include "stopWatch.h"
#include "qfp.h"

 

int      DSOCapturePriv::currentTimeBase=DSOCapture::DSO_TIME_BASE_10MS;
int      DSOCapturePriv::currentVoltageRange=0;
int      DSOCapturePriv::lastRequested=0;
int      DSOCapturePriv::lastAskedSampleCount=0;
int      DSOCapturePriv::triggerValueADC=0;
float    DSOCapturePriv::triggerValueFloat=0;
float     DSOCapturePriv::voltageOffset=0;
DSOCapturePriv::TaskletMode DSOCapturePriv::taskletMode;
FancySemaphore *captureSemaphore=NULL;
static TaskHandle_t captureTaskHandle;

extern StopWatch watch;
CapturedSet DSOCapturePriv::captureSet[2];

/**
 * 
 */
void DSOCapture::initialize()
{
    captureSemaphore=new FancySemaphore;
    xTaskCreate( (TaskFunction_t)DSOCapturePriv::task, "Capture", 200, NULL, DSO_CAPTURE_TASK_PRIORITY, &captureTaskHandle );    
}
/**
 * 
 * @param voltRange
 * @return 
 */
bool     DSOCapture::setVoltageRange(DSOCapture::DSO_VOLTAGE_RANGE voltRange)
{
    watch.ok();
    DSOCapturePriv::currentVoltageRange=voltRange;
    DSOInputGain::setGainRange(vSettings[DSOCapturePriv::currentVoltageRange].gain);
    return true;
}
/**
 * 
 * @return 
 */
DSOCapture::DSO_VOLTAGE_RANGE DSOCapture::getVoltageRange()
{
     return (DSOCapture::DSO_VOLTAGE_RANGE )DSOCapturePriv::currentVoltageRange;
}


/**
 * 
 * @param count
 * @return 
 */
bool DSOCapturePriv::getSamples(CapturedSet **set, int timeoutMs)
{
    if(!captureSemaphore->take(10)) 
    {
        return false;
    }
    *set=DSOCapturePriv::captureSet;
    return true;
}
/**
 * 
 * @return 
 */
void DSOCapturePriv::task(void *a)
{
    xDelay(20);
    while(1)
    {
        switch(taskletMode)
        {
            case  Tasklet_Idle: 
                            xDelay(5);
                            break;
            case  Tasklet_Running:
                           currentTable->tasklet();
                           break;
            case Tasklet_Parking:
                           taskletMode=Tasklet_Idle;
                           break;
            default:
                xAssert(0);
                break;
        }
    }
}

#if 0
/**
 * 
 * @param count
 * @return 
 */
int DSOCapture::oneShotCapture(int count,float *samples,CaptureStats &stats)
{
    xAssert(0);
    DSOCapturePriv::prepareSampling();
    if(!startCapture(count)) return 0;
    CapturedSet *set;
    bool r=    getSamples(&set,10);
    if(!r) return 0;
    
    int toCopy=set->samples;
    if(toCopy>count) toCopy=count;
    
    memcpy(samples,set->data,toCopy*sizeof(float));
    stats=set->stats;
    xDelay(10);
    return toCopy;
    
}
#endif
/**
 * 
 * @param volt
 */
void        DSOCapture::setTriggerValue(float volt)
{
        watch.ok();
        DSOCapturePriv::triggerValueFloat=volt;
        DSOCapturePriv::triggerValueADC=DSOCapturePriv::voltToADCValue(DSOCapturePriv::triggerValueFloat);
}
/**
 * 
 * @param volt
 * @return 
 */
float       DSOCapture::getTriggerValue(void)
{
    return DSOCapturePriv::triggerValueFloat;
}



/**
 * 
 * @param v
 * @param set
 * @return 
 */
int DSOCapturePriv::voltToADCValue(float v)
{    
    float out=v/DSOInputGain::getMultiplier();;
    out+=DSOInputGain::getOffset( INDEX_AC1_DC0());
    return (int)out;    
}

void DSOCapturePriv::InternalStopCapture()
{
    stopCapture();
}

void        DSOCapture::stopCapture()
{       
    

    // wait for the tasklet to be parked
    if(DSOCapturePriv::Tasklet_Running==DSOCapturePriv::taskletMode)
    {
        DSOCapturePriv::taskletMode=DSOCapturePriv::Tasklet_Parking;
    }        
    while(DSOCapturePriv::taskletMode!=DSOCapturePriv::Tasklet_Idle)
    {
        xDelay(1);
    }
    // Now shutdown
    currentTable->stopCapture();
    // Clear interrupts
    uint32_t dummy;
    dummy=ADC1->regs->DR;
    ADC1->regs->SR=0;
    
    controlButtons->updateCouplingState();
    
}


/**
 * 
 * @param count
 * @return 
 */
StopWatch watch;
int DSOCapture::capture(int count,float *volt,CaptureStats &stats)
{
    return DSOCapturePriv::triggeredCapture(count,volt,stats);
}



/**
 * 
 * @param count
 * @param volt
 * @param stats
 * @return 
 */
int DSOCapturePriv::triggeredCapture(int count,float *volt,CaptureStats &stats)
{
    if(taskletMode==DSOCapturePriv::Tasklet_Idle)
    {
        DSOCapturePriv::prepareSampling();
        if(!startCapture(count)) 
            return 0;
    }
                   
    CapturedSet *set;
    bool r=    getSamples(&set,0);
    if(!r) 
    {
        if(watch.elapsed(400))
        {
         //   xAssert(0);
        }
        return 0;
    }
    watch.ok();
    
    if(set->samples<200)
    {
        currentTable->nextCapture(lastRequested);
        return 0;
    }
    InternalStopCapture();
    int toCopy=set->samples;
     if(toCopy>count) toCopy=count;

     memcpy(volt,set->data,toCopy*sizeof(float));
     stats=set->stats;
     //xDelay(10);
     
     return toCopy;
}
/**
 * 
 * @param count
 * @param samples
 * @param waveForm
 * @return 
 */
#if 0 // this one takes ~ 2ms for full sample
bool DSOCapture::captureToDisplay(int count,float *samples,uint8_t *waveForm)
{    
    float gain=vSettings[DSOCapturePriv::currentVoltageRange].displayGain;
    for(int j=0;j<count;j++)
        {
            float v=samples[j]+DSOCapturePriv::voltageOffset;
            v*=(gain*8.)/10.;
            v=DSO_WAVEFORM_HEIGHT/2-v;             
            if(v>DSO_WAVEFORM_HEIGHT) v=DSO_WAVEFORM_HEIGHT;
            if(v<0) v=0;           
            waveForm[j]=(uint8_t)v;
        }
    return true;
}
#else // that one takes ~ 640 us for full samples
        // 590 with qfp
        // 260 with qfp and cast to int

#if 0
    #define DBG(x) x
#else
    #define DBG(...) 
#endif
DBG(uint32_t capcapcap[16];)
DBG(uint32_t capIndex;)
  
        
bool DSOCapture::captureToDisplay(int count,float *samples,uint8_t *waveForm)
{    
    DBG(uint32_t before=micros());
    float gain=vSettings[DSOCapturePriv::currentVoltageRange].displayGain;
    //uint32_t before=micros();
    float offset=(float)(DSO_WAVEFORM_HEIGHT/2)-((float)DSOCapturePriv::voltageOffset*gain*8.)/10.;
    float gain8=QMUL(gain,.8);
    for(int j=0;j<count;j++)
        {
            float v=samples[j];
            v=QMUL(v,gain8);
            v=QADD(offset,-v);     
            int vint=(int)v;
            if(vint>DSO_WAVEFORM_HEIGHT) vint=DSO_WAVEFORM_HEIGHT;
            if(vint<0) vint=0;           
            waveForm[j]=(uint8_t)vint;
        }
    DBG(uint32_t duration=micros()-before);
    DBG(capcapcap[capIndex]=duration);
    DBG(capIndex=(capIndex+1)&0xf);
    return true;
}
#endif

/**
 * 
 * @param count
 * @param samples
 * @param waveForm
 * @return 
 */
int DSOCapture::voltageToPixel(float v)
{    
    float gain=vSettings[DSOCapturePriv::currentVoltageRange].displayGain;
    v*=gain;
    v=120-v;             
    if(v>239) v=239;
    if(v<0) v=0;           
    return (int)v;
}
/**
 * 
 * @param range
 * @return 
 */
float       DSOCapture::getVoltageRangeAsFloat(DSO_VOLTAGE_RANGE range)
{
#define SW(x,y) case   DSO_VOLTAGE_##x: return y;
        
    switch(range)
    {
        SW(GND,0.00001)

        SW(5MV,0.005)
        SW(10MV,0.01)
        SW(20MV,0.02)
        SW(50MV,0.05)    

        SW(100MV,0.1)
        SW(200MV,0.2)
        SW(500MV,0.5)        
        SW(1V,1)
        SW(2V,2)
        SW(5V,5)
    }
    xAssert(0);
}
/**
 * 
 * @param range
 * @return 
 */
int       DSOCapture::getVoltageRangeIndex(DSO_VOLTAGE_RANGE range)
{
#undef SW
#define SW(x,y) case   DSO_VOLTAGE_##x: return y;
        
    switch(range)
    {
        SW(GND,0)
        SW(5MV,3)
        SW(10MV,4)
        SW(20MV,5)
        SW(50MV,6)    

        SW(100MV,7)
        SW(200MV,8)
        SW(500MV,9)        
        SW(1V,10)
        SW(2V,11)
        SW(5V,12)
    }
    xAssert(0);
}

/**
 * 
 * @return 
 */
const char *DSOCapture::getVoltageRangeAsText()
{
    return vSettings[DSOCapturePriv::currentVoltageRange].name;
}
/**
 * 
 */
void        DSOCapture::clearCapturedData()
{
    //adc->clearCapturedData();
}
/**
 * 
 * @param mode
 */
void        DSOCapture::setTriggerMode(TriggerMode mode)
{
    watch.ok();
    DSOADC::TriggerMode adcMode;
    switch(mode)
    {
#define CAP2ADC(xx)         case DSOCapture::xx: adcMode=DSOADC::xx;break;
        
        CAP2ADC(Trigger_Rising);
        CAP2ADC(Trigger_Falling);
        CAP2ADC(Trigger_Both);
        CAP2ADC(Trigger_Run);
        default:
            xAssert(0);
            break;
    }
    DSOCapturePriv::InternalStopCapture();
    adc->setTriggerMode(adcMode);
}
/**
 * 
 * @return 
 */
DSOCapture::TriggerMode DSOCapture::getTriggerMode()
{
    DSOADC::TriggerMode adcMode=adc->getTriggerMode();
    DSOCapture::TriggerMode mode;
   #define _CAP2ADC(xx)         case DSOADC::xx: mode=DSOCapture::xx;break;
    switch(adcMode)  
    {
        _CAP2ADC(Trigger_Rising);
        _CAP2ADC(Trigger_Falling);
        _CAP2ADC(Trigger_Both);
        _CAP2ADC(Trigger_Run);
        default:
            xAssert(0);
            break;
    }
    return mode;
}


/**
 * 
 * @param volt
 */
void  DSOCapture::setVoltageOffset(float volt)
{
    watch.ok();
    DSOCapturePriv::voltageOffset=volt;
}
/**
 * 
 * @return 
 */
float DSOCapture::getVoltageOffset()
{
    return DSOCapturePriv::voltageOffset;
}

int DSOCapture::voltToADCValue(float v)
{
    return DSOCapturePriv::voltToADCValue(v);
}
/**
 * \fn getMaxVoltageValue
 * \brief return the max absolute voltage for the current selected voltage range
 * It the actual value is higher than max it means either ADC saturates or we can't display it
 * @return 
 */
float        DSOCapture::getMaxVoltageValue()
{
     // we want to have less than 80% pixels= 100 (i.e. half screen), else it means saturation
   
    float gain=vSettings[DSOCapturePriv::currentVoltageRange].displayGain;
    float v=116./gain;
    return v;
}
float        DSOCapture::getMinVoltageValue()
{
    // Same but for 2 blocks i.e. 2*24=48 pixel
   
    float gain=vSettings[DSOCapturePriv::currentVoltageRange].displayGain;
    float v=40./gain;
    return v;
}
/**
 * 
 * @param timeBase
 * @return 
 */
int         DSOCapture::timeBaseToFrequency(DSOCapture::DSO_TIME_BASE timeBase)
{
#define CASE(x,y) case DSO_TIME_BASE_##x: return y;break;
    
    switch(timeBase)
    {
        CASE(5US,   500*1000) 
        CASE(10US,  100*1000) 
        CASE(25US,  40*1000) 
        CASE(50US,  20*1000) 
        CASE(100US, 10*1000) 
        CASE(500US, 2*1000) 
        CASE(1MS,   1000) 
        CASE(5MS,   200) 
        CASE(10MS,  100) 
        CASE(50MS,  20) 
        CASE(100MS, 10) 
        CASE(500MS, 2) 
        CASE(1S,    1)     
    }
    xAssert(0);
    return 0;
}

// EOF


