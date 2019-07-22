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

static int transformDma(int16_t *in, float *out,int count, VoltageSettings *set,int expand,CaptureStats &stats, float triggerValue, DSOADC::TriggerMode mode)
{
   if(!count) return false;
   stats.xmin=200;
   stats.xmax=-200;
   stats.avg=0;
   int ocount=(count*4096)/expand;
   if(ocount>240)
   {
       ocount=240;
   }
   ocount&=0xffe;
   int dex=0;
   
   // First
   float f;
   {
       f=(float)in[0]; 
       f-=set->offset;
       f*=set->multiplier;       
       if(f>stats.xmax) stats.xmax=f;
       if(f<stats.xmin) stats.xmin=f;       
       out[0]=f; // Unit is now in volt
       stats.avg+=f;
       dex+=expand;
   }
   
   // med
   //if(stats.trigger==-1)
   {   
    for(int i=1;i<ocount;i++)
    {

        f=*(in+(dex/4096));
        f-=set->offset;
        f*=set->multiplier;
        if(f>stats.xmax) stats.xmax=f;
        if(f<stats.xmin) stats.xmin=f;       
        out[i]=f; // Unit is now in volt

        if(stats.trigger==-1)
        {
             if(mode!=DSOADC::Trigger_Rising)
                 if(out[i-1]<triggerValue&&out[i]>=triggerValue) stats.trigger=i;
             if(mode!=DSOADC::Trigger_Falling)
                 if(out[i-1]>triggerValue&&out[i]<=triggerValue) stats.trigger=i;
        }

        stats.avg+=f;
        dex+=expand;
    }   
   }
   stats.avg/=count;
   return ocount;
}

/**
 * 
 * @return 
 */
bool DSOCapturePriv::prepareSamplingDma()
{
  //     
    const TimeSettings *set= tSettings+currentTimeBase;
    return adc->prepareDMASampling(set->rate,set->prescaler);
}
/**
 * 
 * @return 
 */
DSOCapture::DSO_TIME_BASE DSOCapturePriv::getTimeBaseDma()
{
    return (DSOCapture::DSO_TIME_BASE)currentTimeBase;
}


/**
 * 
 */
void DSOCapturePriv::stopCaptureDma()
{
    
    adc->stopDmaCapture();
    
}
/**
 * 
 * @return 
 */
const char *DSOCapturePriv::getTimeBaseAsTextDma()
{
   return tSettings[currentTimeBase].name;
}


/**
 * 
 * @param count
 * @return 
 */
bool       DSOCapturePriv:: startCaptureDma (int count)
{
    int ex=count*tSettings[currentTimeBase].expand4096;
    return adc->startDMASampling(ex);
}
/**
 * 
 * @param count
 * @return 
 */
bool       DSOCapturePriv:: startCaptureDmaTrigger (int count)
{
    
    int ex=count;
    ex=count*tSettings[currentTimeBase].expand4096;
    lastRequested=ex/4096;
    return adc->startDMATriggeredSampling(ex,triggerValueADC);
}
/**
 * 
 * @return 
 */
bool DSOCapturePriv::nextCapture()
{
    adc->resetStats();    
    currentTable->nextCapture(lastRequested);
    return true;        
}
/**
 * 
 * @return 
 */
bool DSOCapturePriv::taskletDmaCommon(const bool trigger)
{
    
    FullSampleSet fset; // Shallow copy
    int16_t *p;
    
    int currentVolt=currentVoltageRange; // use a local copy so that it does not change in the middle
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
    if(trigger)
    {
        bool triggerFound=refineCapture(fset);
        if(!triggerFound)
        {
          if(trigger)
          {
                nextCapture();                
                return false;
          }
        }
        set->stats.trigger=120; // right in the middle
    }else
    {
        set->stats.trigger=120; // right in the middle
    }
    
    int     scale=vSettings[currentVolt].inputGain;
    int    expand=tSettings[currentTime].expand4096;
    
    float *data=set->data;    

    p=((int16_t *)fset.set1.data);

    set->samples=transformDma(
                                    p,
                                    data,
                                    fset.set1.samples,
                                    vSettings+currentVolt,
                                    expand,
                                    set->stats,
                                    triggerValueFloat,
                                    adc->getTriggerMode());      
        
    int fint=computeFrequency(fset.set1.samples,fset.set1.data);
    if(fint)
    {
            float f=fint;    
            f=(float)(tSettings[currentTimeBase].fqInHz)*1000./f;
            set->stats.frequency=f;
    }else
         set->stats.frequency=0;
    // Data ready!
    captureSemaphore->give();
    return true;
}
/**
 * 
 * @return 
 */
bool DSOCapturePriv::taskletDma()
{
    
    return taskletDmaCommon(true);

}
//--
bool        DSOCapturePriv::nextCaptureDmaTrigger(int count)
{
    return startCaptureDmaTrigger(count);
}
bool        DSOCapturePriv::nextCaptureDma(int count)
{
    return startCaptureDma(count);
}

/**
 * 
 * @return 
 */
bool DSOCapturePriv::taskletDmaRunning()
{
     return taskletDmaCommon(false);
}
#if 0
/**
 * 
 * @param trigger
 */
void DSOCapturePriv::reigniteDmaCommon(const bool trigger)
{
    adc->restartDmaTriggerCapture();
}
#endif
// EOF
