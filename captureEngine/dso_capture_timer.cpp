/***************************************************
 STM32 duino based firmware for DSO SHELL/150
 *  * GPL v2
 * (c) mean 2019 fixounet@free.fr
 ****************************************************/
#include "dso_global.h"
#include "dso_adc.h"
#include "dso_capture.h"
#include "dso_capture_priv.h"
#include "dso_adc_gain.h"
#include "DSO_config.h"

extern int transformDmaExact(int dc0_ac1,int16_t *in, float *out,int count, CaptureStats &stats, float triggerValue, DSOADC::TriggerMode mode,int swing);

/**
 * 
 * @return 
 */
bool DSOCapturePriv::prepareSamplingTimer()
{
    return adc->prepareTimerSampling(timerBases[currentTimeBase].fq);
}

/**
 * 
 * @return 
 */
DSOCapture::DSO_TIME_BASE DSOCapturePriv::getTimeBaseTimer()
{
    return (DSOCapture::DSO_TIME_BASE)(currentTimeBase+DSO_TIME_BASE::SLOWER_FAST_MODE+1);
}

/**
 * 
 */
void DSOCapturePriv::stopCaptureTimer()
{
    adc->stopDmaCapture();
    
}
/**
 * 
 * @return 
 */

const char *DSOCapturePriv::getTimeBaseAsTextTimer()
{
    return timerBases[currentTimeBase].name;
}

//--
bool        DSOCapturePriv::nextCaptureTimer(int count)
{
    return startCaptureTimer(count);
}
bool        DSOCapturePriv::nextCaptureTimerTrigger(int count)
{
    return startCaptureTimerTrigger(count);
}
/**
 * 
 * @param count
 * @return 
 */
bool       DSOCapturePriv:: startCaptureTimer (int count)
{    
    return adc->startTimerSampling(count);
}
/**
  * 
 */
bool       DSOCapturePriv:: startCaptureTimerTrigger (int count)
{
    return adc->startTriggeredTimerSampling(count,triggerValueADC);
}
/**
 */
bool DSOCapturePriv::taskletTimer()
{    
    FullSampleSet fset; // Shallow copy
    int16_t *p;    
    int currentTime=currentTimeBase;

    if(!adc->getSamples(fset))
          return false;
                
    if(!fset.set1.samples)
    {
        nextCapture();
        return false;
    }

    CapturedSet *set=captureSet;
    
    set->stats.trigger=-1;     
    set->stats.frequency=-1;
    float *data=set->data;    
    p=((int16_t *)fset.set1.data);
#if 0    
    if(trigger)
    {
        bool triggerFound=refineCapture(fset,needed);
        if(!triggerFound)
        {
            nextCapture();                
            return false;
        }        
    }
#endif    
    set->stats.trigger=120; // right in the middle
    p=((int16_t *)fset.set1.data);
    set->samples=transformDmaExact(      INDEX_AC1_DC0(),
                                    p,
                                    data,
                                    fset.set1.samples,
                                    set->stats,
                                    triggerValueFloat,
                                    adc->getTriggerMode(),
                                    vSettings[DSOCapturePriv::currentVoltageRange].maxSwing
                                    );      
        
    int fint=computeFrequency(fset.set1.samples,fset.set1.data);
    if(fint)
    {
            float f=fint;    
            f=(float)(tSettings[currentTimeBase].fqInHz)*1000./f;
            set->stats.frequency=f;
    }else  
    {
         set->stats.frequency=0;
    }
    // Data ready!
    captureSemaphore->give();
    return true;
}

// EOF