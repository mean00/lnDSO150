/***************************************************
 STM32 duino based firmware for DSO SHELL/150
 *  * GPL v2
 * (c) mean 2019 fixounet@free.fr
 ****************************************************/
#include "dso_global.h"
#include "dso_adc.h"
#include "dso_capture.h"
#include "dso_capture_priv.h"
#include "transform.h"
#include "DSO_config.h"

static int      canary1=0xabcde01234;
static int      currentTimeBase=DSOCapture::DSO_TIME_BASE_10MS;
static int      currentVoltageRange=0;
static bool     captureFast=true;
static int      canary2=0xabcde01234;
static int      triggerValueADC=0;
static float    triggerValueFloat=0;
extern DSOADC   *adc;
xBinarySemaphore *captureSemaphore;
static TaskHandle_t captureTaskHandle;
static float     voltageOffset=0;

CapturedSet captureSet[2];




/**
 * 
 */
void DSOCapture::initialize()
{
    captureSemaphore=new xBinarySemaphore;
    xTaskCreate( (TaskFunction_t)DSOCapture::task, "Capture", 200, NULL, DSO_CAPTURE_TASK_PRIORITY, &captureTaskHandle );    
}
/**
 * 
 * @return 
 */
DSOCapture::DSO_TIME_BASE DSOCapture::getTimeBase()
{
    if(captureFast) 
        return (DSOCapture::DSO_TIME_BASE)currentTimeBase;
    return (DSOCapture::DSO_TIME_BASE)(currentTimeBase+DSO_TIME_BASE::DSO_TIME_BASE_5MS);
}
/**
 * 
 * @param voltRange
 * @return 
 */
bool     DSOCapture::setVoltageRange(DSOCapture::DSO_VOLTAGE_RANGE voltRange)
{
    currentVoltageRange=voltRange;
    controlButtons->setInputGain(vSettings[currentVoltageRange].inputGain);
    return true;
}
/**
 * 
 * @return 
 */
DSOCapture::DSO_VOLTAGE_RANGE DSOCapture::getVoltageRange()
{
     return (DSOCapture::DSO_VOLTAGE_RANGE )currentVoltageRange;
}


/**
 * 
 * @param timeBase
 * @return 
 */

bool     DSOCapture::setTimeBase(DSOCapture::DSO_TIME_BASE timeBase)
{
    if(timeBase>DSO_TIME_BASE_MAX)
    {
        xAssert(0);
    }
    if(timeBase<DSO_TIME_BASE::DSO_TIME_BASE_5MS) // fast mode
    {
        captureFast   =true;
        currentTimeBase=timeBase;
        
    }else
    {
        captureFast=false;
        currentTimeBase=timeBase-DSO_TIME_BASE::DSO_TIME_BASE_5MS;
    }
}
/**
 * 
 * @param count
 * @return 
 */
bool     DSOCapture::prepareSampling ()
{
    if(captureFast)
    {
        //     
        const TimeSettings *set= tSettings+currentTimeBase;
        return adc->prepareDMASampling(set->rate,set->prescaler);
    }else
    {
        return adc->prepareTimerSampling(timerBases[currentTimeBase].fq);
    }
}
/**
 * 
 * @return 
 */
bool     DSOCapture::startSampling (int count)
{
    controlButtons->updateCouplingState();
    if(captureFast)
    {
        int ex=count*tSettings[currentTimeBase].expand4096;
        return adc->startDMASampling(ex);
    }
    return adc->startTimerSampling(count);
}
/**
 * 
 * @return 
 */
bool     DSOCapture::startTriggerSampling (int count)
{
    triggerValueADC=voltToADCValue(triggerValueFloat);
    int ex=count;
    if(captureFast)
    {
        ex=count*tSettings[currentTimeBase].expand4096;
        //return adc->startTriggeredTimerSampling(ex,triggerValueADC);
        return adc->startDMATriggeredSampling(ex,triggerValueADC);
    }
    return adc->startTriggeredTimerSampling(ex,triggerValueADC);
    
}
/**
 * 
 * @param count
 * @return 
 */
bool DSOCapture::getSamples(CapturedSet **set, int timeoutMs)
{
    if(!captureSemaphore->take(10)) return false;
    *set=captureSet;
    return true;
}


/**
 * 
 * @return 
 */
void DSOCapture::task(void *a)
{
    xDelay(20);
    FullSampleSet fset; // Shallow copy
    int16_t *p;
    while(1)
    {
        int currentVolt=currentVoltageRange; // use a local copy so that it does not change in the middle
        int currentTime=currentTimeBase;
        if(!adc->getSamples(fset))
            continue;

        if(!fset.set1.samples)
        {
            continue;
        }
        int scale=vSettings[currentVolt].inputGain;
        int expand=4096;
        if(captureFast)            
        {
            expand=tSettings[currentTime].expand4096;
        }
        CapturedSet *set=captureSet;
        float *data=set->data;    
        if(fset.shifted)
            p=((int16_t *)fset.set1.data)+1;
        else
            p=((int16_t *)fset.set1.data);

        set->samples=transform(
                                        p,
                                        data,
                                        fset.set1.samples,
                                        vSettings+currentVolt,
                                        expand,
                                        set->stats,
                                        triggerValueFloat,
                                        adc->getTriggerMode());
        if(fset.set2.samples)
        {
            CaptureStats otherStats;
            if(fset.shifted)
                p=((int16_t *)fset.set2.data)+1;
            else
                p=((int16_t *)fset.set2.data);
            int sample2=transform(
                                        p,
                                        data+set->samples,
                                        fset.set2.samples,
                                        vSettings+currentVolt,
                                        expand,
                                        otherStats,
                                        triggerValueFloat,
                                        adc->getTriggerMode());                
            if(set->stats.trigger==-1 && otherStats.trigger!=-1) 
            {
                set->stats.trigger=otherStats.trigger+set->samples;
            }            
            set->stats.avg= (set->stats.avg*set->samples+otherStats.avg*fset.set2.samples)/(set->samples+fset.set2.samples);
            set->samples+=sample2;
            if(otherStats.xmax>set->stats.xmax) set->stats.xmax=otherStats.xmax;
            if(otherStats.xmin<set->stats.xmin) set->stats.xmin=otherStats.xmin;
            
        }
        set->stats.frequency=-1;
        
        float f=computeFrequency(fset.shifted,fset.set1.samples,fset.set1.data);
        if(captureFast)
        {
            f=(float)(tSettings[currentTimeBase].fqInHz)*1000./f;
        }else
        {
            f=((float)timerBases[currentTimeBase].fq)*1000./f;
        }
        set->stats.frequency=f;
        
        // Data ready!
        captureSemaphore->give();
    }

}


/**
 * 
 * @param count
 * @return 
 */
int DSOCapture::oneShotCapture(int count,float *samples,CaptureStats &stats)
{
    prepareSampling();
    if(!startSampling(count)) return 0;
    CapturedSet *set;
    bool r=    getSamples(&set,500);
    if(!r) return 0;
    
    int toCopy=set->samples;
    if(toCopy>count) toCopy=count;
    
    memcpy(samples,set->data,toCopy*sizeof(float));
    stats=set->stats;
    xDelay(10);
    return toCopy;
    
}
/**
 * 
 * @param volt
 */
void        DSOCapture::setTriggerValue(float volt)
{
        triggerValueFloat=volt;
        triggerValueADC=voltToADCValue(triggerValueFloat);
}
/**
 * 
 * @param volt
 * @return 
 */
float       DSOCapture::getTriggerValue(void)
{
    return triggerValueFloat;
}



/**
 * 
 * @param v
 * @param set
 * @return 
 */
int DSOCapture::voltToADCValue(float v)
{
    VoltageSettings *set=&(vSettings[currentVoltageRange]);
    float out=v/set->multiplier;
    out+=set->offset;
    return (int)out;    
}

typedef enum InternalCaptureState
{
    captureStateIdle,
    captureStateArmed
};
InternalCaptureState captureState=captureStateIdle;

/**
 * 
 */
void DSOCapture::stopCapture()
{
    if(captureFast)
        adc->stopDmaCapture();
    else
        adc->stopTimeCapture();
    captureState=captureStateIdle;
    controlButtons->updateCouplingState();
}
/**
 * 
 * @param count
 * @return 
 */
int DSOCapture::triggeredCapture(int count,float *volt,CaptureStats &stats)
{
    if(captureState==captureStateIdle)
    {
        prepareSampling();
        if(!startTriggerSampling(count)) return 0;
        captureState=captureStateArmed;
    }
                   
    CapturedSet *set;
    bool r=    getSamples(&set,0);
    if(!r) 
        return 0;
    captureState=captureStateIdle;
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
bool DSOCapture::captureToDisplay(int count,float *samples,uint8_t *waveForm)
{    
    float gain=vSettings[currentVoltageRange].displayGain;
    for(int j=0;j<count;j++)
        {
            float v=samples[j]+voltageOffset;
            v*=(gain*8.)/10.;
            v=DSO_WAVEFORM_HEIGHT/2-v;             
            if(v>DSO_WAVEFORM_HEIGHT) v=DSO_WAVEFORM_HEIGHT;
            if(v<0) v=0;           
            waveForm[j]=(uint8_t)v;
        }
    return true;
}

/**
 * 
 * @param count
 * @param samples
 * @param waveForm
 * @return 
 */
int DSOCapture::voltageToPixel(float v)
{    
    float gain=vSettings[currentVoltageRange].displayGain;
    v*=gain;
    v=120-v;             
    if(v>239) v=239;
    if(v<0) v=0;           
    return (int)v;
}
/**
 * 
 * @return 
 */
const char *DSOCapture::getTimeBaseAsText()
{
    if(captureFast)
    {
        return tSettings[currentTimeBase].name;
    }
    return timerBases[currentTimeBase].name;
}
/**
 * 
 * @return 
 */
const char *DSOCapture::getVoltageRangeAsText()
{
    return vSettings[currentVoltageRange].name;
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
    DSOADC::TriggerMode adcMode;
    switch(mode)
    {
#define CAP2ADC(xx)         case DSOCapture::xx: adcMode=DSOADC::xx;break;
        
        CAP2ADC(Trigger_Rising);
        CAP2ADC(Trigger_Falling);
        CAP2ADC(Trigger_Both);
        default:
            xAssert(0);
            break;
    }
    stopCapture();
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
    voltageOffset=volt;
}
/**
 * 
 * @return 
 */
float DSOCapture::getVoltageOffset()
{
    return voltageOffset;
}
#include "dso_capture2.cpp"
// EOF
