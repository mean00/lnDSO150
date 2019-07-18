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

int      DSOCapturePriv::currentTimeBase=DSOCapture::DSO_TIME_BASE_10MS;
int      DSOCapturePriv::currentVoltageRange=0;
int      DSOCapturePriv::lastRequested=0;
int      DSOCapturePriv::triggerValueADC=0;
float    DSOCapturePriv::triggerValueFloat=0;
float     DSOCapturePriv::voltageOffset=0;
FancySemaphore *captureSemaphore=NULL;
static TaskHandle_t captureTaskHandle;


typedef enum InternalCaptureState
{
    captureStateIdle,
    captureStateArmed
};
InternalCaptureState captureState=captureStateIdle; 

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
    DSOCapturePriv::currentVoltageRange=voltRange;
    controlButtons->setInputGain(vSettings[DSOCapturePriv::currentVoltageRange].inputGain);
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
bool DSOCapture::getSamples(CapturedSet **set, int timeoutMs)
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
 * @param set
 */
bool DSOCapturePriv::refineCapture(FullSampleSet &set)
{
         // Try to find the trigger, we have ADC_INTERNAL_BUFFER_SIZE samples coming in, we want requestSample out..
        uint16_t *p=(uint16_t *)set.set1.data;
        int count=set.set1.samples;
        
        int found=-1;
        int asked=240;
        int start=lastRequested/2;
        int end=count-lastRequested/2;
        
        switch(adc->getTriggerMode())
        {
            case Trigger_Run:
                found=0;
                break;
            case Trigger_Rising:
                for(int i=start;i<end;i++)
                {
                    if(p[i]<triggerValueADC && p[i+1]>=triggerValueADC) 
                    {
                        found=i;
                        break;
                    }
                }
                break;
            case Trigger_Falling :
                for(int i=start;i<end;i++)
                {
                    if(p[i]>triggerValueADC && p[i+1]<=triggerValueADC) 
                    {
                        found=i;
                        break;
                    }
                }
                 break;
            case Trigger_Both:
                // Tricky !
                for(int i=start;i<end;i++)
                {
                    if(p[i]>triggerValueADC && p[i+1]<=triggerValueADC) 
                    {
                        found=i;
                        break;
                    }
                    if(p[i]<triggerValueADC && p[i+1]>=triggerValueADC) 
                    {
                        found=i;
                        break;
                    }
                }
                break;
        }
        if(found==-1)
        {
            set.set1.samples=lastRequested;
            return false;
        }
        set.set1.data+=found-lastRequested/2;
        set.set1.samples=lastRequested;        
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
#warning FIXME : Waste of time        
        if(captureState==captureStateIdle) // 
            xDelay(1);
        else
            currentTable->tasklet();
    }

}


/**
 * 
 * @param count
 * @return 
 */
int DSOCapture::oneShotCapture(int count,float *samples,CaptureStats &stats)
{
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
/**
 * 
 * @param volt
 */
void        DSOCapture::setTriggerValue(float volt)
{
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
    VoltageSettings *set=&(vSettings[currentVoltageRange]);
    float out=v/set->multiplier;
    out+=set->offset;
    return (int)out;    
}



void        DSOCapture::stopCapture()
{
    currentTable->stopCapture();
    captureState=captureStateIdle;
    controlButtons->updateCouplingState();

}


/**
 * 
 * @param count
 * @return 
 */
StopWatch watch;
int DSOCapture::triggeredCapture(int count,float *volt,CaptureStats &stats)
{
    if(captureState==captureStateIdle)
    {
        DSOCapturePriv::prepareSampling();
        if(!startCapture(count)) return 0;
        captureState=captureStateArmed;
    }
                   
    CapturedSet *set;
    bool r=    getSamples(&set,0);
    if(!r) 
    {
        if(watch.elapsed(400))
            xAssert(0);
        return 0;
    }
    watch.ok();
    
    if(set->samples<200) xAssert(0);
    
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

// EOF


