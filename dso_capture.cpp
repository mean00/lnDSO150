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


static int      canary1=0xabcde01234;
static int      currentTimeBase=DSOCapture::DSO_TIME_BASE_10MS;
static int      currentVoltageRange=0;
static int      lastRequested=0;
static bool     captureFast=true;
static int      canary2=0xabcde01234;
static int      triggerValueADC=0;
static float    triggerValueFloat=0;
extern DSOADC   *adc;
xBinarySemaphore *captureSemaphore;
static TaskHandle_t captureTaskHandle;
static float     voltageOffset=0;

CapturedSet captureSet[2];

#include "dso_capture_indirection.h"

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
 * @param set
 */
bool DSOCapture::refineCapture(FullSampleSet &set)
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
            case Trigger_Rising:
                for(int i=start;i<end;i++)
                {
                    if(p[2*i]<triggerValueADC && p[2*i+2]>=triggerValueADC) 
                    {
                        found=i;
                        break;
                    }
                }
                break;
            case Trigger_Falling :
                for(int i=start;i<end;i++)
                {
                    if(p[2*i]>triggerValueADC && p[2*i+2]<=triggerValueADC) 
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
                    if(p[2*i]>triggerValueADC && p[2*i+2]<=triggerValueADC) 
                    {
                        found=i;
                        break;
                    }
                    if(p[2*i]<triggerValueADC && p[2*i+2]>=triggerValueADC) 
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
void DSOCapture::task(void *a)
{
    xDelay(20);
    while(1)
    {
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
    prepareSampling();
    if(!startCapture(count)) return 0;
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
int DSOCapture::triggeredCapture(int count,float *volt,CaptureStats &stats)
{
    if(captureState==captureStateIdle)
    {
        prepareSampling();
        if(!startCapture(count)) return 0;
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


