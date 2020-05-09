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
#include "dso_adc_gain.h"

/**
 * 
 * @param dc0_ac1
 * @param in
 * @param out
 * @param count
 * @param stats
 * @param triggerValue
 * @param mode
 * @param swing
 * @return 
 */
static int transformDmaExact(int dc0_ac1,int16_t *in, float *out,int count, CaptureStats &stats, float triggerValue, DSOADC::TriggerMode mode,int swing)
{    
   if(!count) return false;
   stats.xmin=200;
   stats.xmax=-200;
   stats.saturation=false;
   stats.avg=0;
   int dex=0;
   float offset,multiplier;   
   offset=DSOInputGain::getOffset(dc0_ac1);
   multiplier=DSOInputGain::getMultiplier();
   // First
   float f;
   {
       int v=in[0];
       if(v<swing) stats.saturation=true;
       if(v>(4096-swing)) stats.saturation=true;
       f=(float)v; 
       f-=offset;
       f*=multiplier;       
       if(f>stats.xmax) stats.xmax=f;
       if(f<stats.xmin) stats.xmin=f;       
       out[0]=f; // Unit is now in volt
       stats.avg+=f;
       dex++;
   }
   
   // med
   //if(stats.trigger==-1)
   {   
    for(int i=1;i<count;i++)
    {

        f=*(in+dex);
        f-=offset;
        f*=multiplier;
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
        dex++;
    }   
   }
   stats.avg/=(float)count;
   return count;
}

static int transformDma(int dc0_ac1,int16_t *in, float *out,int count, int expand,CaptureStats &stats, float triggerValue, DSOADC::TriggerMode mode,int swing)
{    
   if(!count) return false;
   if(expand==4096)
       return transformDmaExact(dc0_ac1,in,out,count,stats,triggerValue,mode,swing);
   stats.xmin=200;
   stats.xmax=-200;
   stats.saturation=false;
   stats.avg=0;
   int ocount=(count*4096)/expand;
   if(ocount>240)
   {
       ocount=240;
   }
   ocount&=0xffe;
   int dex=0;
   float offset,multiplier;   
   offset=DSOInputGain::getOffset(dc0_ac1);
   multiplier=DSOInputGain::getMultiplier();
   // First
   float f;
   {
       int v=in[0];
       if(v<swing) stats.saturation=true;
       if(v>(4096-swing)) stats.saturation=true;
       f=(float)v; 
       f-=offset;
       f*=multiplier;       
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
        f-=offset;
        f*=multiplier;
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
   stats.avg/=(float)ocount;
   return ocount;
}
void swapADCs(int nb, uint16_t *data)
{    
    int nbWord=nb/2;   
    uint16_t swap;
    for(int i=0;i<nbWord;i++)
    {
        swap=*data;
        *data=data[1];
        data[1]=swap;
        data+=2;
    }
}
/**
 * 
 * @return 
 */
bool DSOCapturePriv::prepareSamplingDma()
{
  //     
    const TimeSettings *set= tSettings+currentTimeBase;
    if(set->dual)
        return adc->prepareDualDMASampling(DSO_INPUT_PIN,set->rate,set->prescaler);
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
    lastRequested=ex/4096;
    lastAskedSampleCount=count;
    if(IS_CAPTURE_DUAL())
        return adc->startDualDMASampling (DSO_INPUT_PIN, lastRequested);
    else
        return adc->startDMASampling(lastRequested);
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
    lastAskedSampleCount=count;
    xAssert(lastRequested>0);
    xAssert(lastRequested<ADC_INTERNAL_BUFFER_SIZE);
    return adc->startDMATriggeredSampling(lastRequested,triggerValueADC);
}
/**
 * 
 * @return 
 */
bool DSOCapturePriv::nextCapture()
{
    currentTable->nextCapture(lastAskedSampleCount);
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
    int     expand=tSettings[currentTime].expand4096;
    
    float *data=set->data;    

    p=((int16_t *)fset.set1.data);
    if(IS_CAPTURE_DUAL() )
        swapADCs(fset.set1.samples,(uint16_t *)p);
    set->samples=transformDma(      INDEX_AC1_DC0(),
                                    p,
                                    data,
                                    fset.set1.samples,
                                    expand,
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