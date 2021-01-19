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
#include  "qfp.h"
static void swapADCs_c(int nb, uint16_t *data)
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
static void swapADCs_asm(int nb, uint16_t *data)  __attribute__( ( naked ) );
/**
 *   Convert 8 samples at a time
 *    aa bb cc dd ee ff gg hh => bb aa dd cc ff ee hh gg
 * 
 */
static void swapADCs_asm(int nb, uint16_t *data) 
{    
    __asm volatile (    
            "   push {r2}         \n"
            "   lsr r0,#3       \n" //4x 32 bits at a time
            "nxt:               \n"
            "   ldr r2,[r1]     \n"
            "   rev r2,r2       \n"
            "   rev16 r2,r2     \n"
            "   str r2,[r1]     \n"    
    
            "   add r1,r1,#4    \n"
            "   ldr r2,[r1]     \n"
            "   rev r2,r2       \n"
            "   rev16 r2,r2     \n"
            "   str r2,[r1]     \n"    

                "   add r1,r1,#4    \n"
            "   ldr r2,[r1]     \n"
            "   rev r2,r2       \n"
            "   rev16 r2,r2     \n"
            "   str r2,[r1]     \n"    

            "   add r1,r1,#4    \n"
            "   ldr r2,[r1]     \n"
            "   rev r2,r2       \n"
            "   rev16 r2,r2     \n"
            "   str r2,[r1]     \n"    

    
    
            "   add r1,r1,#4    \n"
            "   sub r0,r0,#1    \n"
            "   cmp r0,#0       \n"
            "   bne nxt         \n" 
            "   pop {r2}        \n"    
            "   mov pc,lr       \n" 
            : : : "memory");
}

static void swapADCs(int nb, uint16_t *data)
{    
    int ex=nb&7;
    int r=(nb>>3)<<3;
    swapADCs_asm(r,data);    
    if(ex)
        swapADCs_c(ex,data+r);
}
/**
 * 
 * @param set
 */
bool DSOCapturePriv::refineCapture(FullSampleSet &set,int needed)
{
         // Try to find the trigger, we have ADC_INTERNAL_BUFFER_SIZE samples coming in, we want requestSample out..
        uint16_t *p=(uint16_t *)set.set1.data;
        int count=set.set1.samples;        
        static int found=-1;
        int start=needed/2;  // try to center the output if we can
        int end=count-needed/2;
        xAssert(end>start);
        
        switch(adc->getActualTriggerMode())
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
                xAssert(0); 
                break;
        }
        if(found==-1)
        {     
            set.set1.samples=needed; // just grab the N first 
            return false;
        }
        // center on the trigger
        int offset=(found-needed/2)&0xfffe;
        set.set1.data+=offset; // must be even, else we'll swap them wrong
        set.set1.samples=needed;        
        return true;
}
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

/**
 STM32F1 / 128 Mhz => 
 * Normal : 240 cycle =             880 us
 * Taking min max out of the loop : 570 us
 * Taking avg out of the loop       460 us
 * Searching trigger in integer     440 us
 
 */


#if 0
    #define DBG(x) x;
#else
    #define DBG(x)
#endif
DBG(uint32_t poppop[16])
DBG(uint32_t poppopIndex=0)
        
        
void checkAvgMinMax(int count, int16_t *in,CaptureStats &stats,int swing,float offset,float multiplier)        
{
   
   float f;   
   // Search min/max
   int xmin=4096*2;
   int xmax=-1;
   int sum=0;
   
   for(int i=0;i<count;i++)
   {
       int v=in[i];
       sum+=v;
       if(v>xmax) xmax=v;
       if(v<xmin) xmin=v;
       if(v<swing)        stats.saturation=true;
       if(v>(4096-swing)) stats.saturation=true;
   }
   // compute avg
   f=(float)sum;
   f/=count;
   f-=offset;
   f*=multiplier;     
   stats.avg=f;
   
   // Compute min max
   f=(float)xmax;
   f=QSUB(f,offset);
   f=QMUL(f,multiplier);
   stats.xmax=f;
     
   f=(float)xmin;
   f=QSUB(f,offset);
   f=QMUL(f,multiplier);
    stats.xmin=f;
            
}
        
int transformDmaExact(int dc0_ac1,int16_t *in, float *out,int count, CaptureStats &stats, float triggerValue, DSOADC::TriggerMode mode,int swing)
{    
   if(!count) return false;
   DBG(uint32_t start=micros());
   stats.xmin=200;
   stats.xmax=-200;
   stats.saturation=false;
   stats.avg=0;
   int dex=0;
   float offset,multiplier;   
   float f;   
   offset=DSOInputGain::getOffset(dc0_ac1);
   multiplier=DSOInputGain::getMultiplier();
   
   checkAvgMinMax(count,in,stats,swing,offset,multiplier);
  
   // med
   {   
    for(int i=0;i<count;i++)
    {

        f=*(in+i);
        f=QSUB(f,offset);
        f=QMUL(f,multiplier);
        out[i]=f; // Unit is now in volt        
    }   
   }
    // Search for trigger
     if(stats.trigger==-1)
     {
         float t=triggerValue;
         t=t/multiplier;
         t+=offset;
         
         int tint=(int)t; // Check it in integer, faster
          if(mode!=DSOADC::Trigger_Rising)
          {
               for(int i=1;i<count && stats.trigger==-1;i++)
                    if(in[i-1]<tint&&in[i]>=tint) stats.trigger=i;
          }
         // The else is ***NOT** Missing here
          if(mode!=DSOADC::Trigger_Falling)
          {
               for(int i=1;i<count && stats.trigger==-1;i++)
                   if(in[i-1]>tint&&in[i]<=tint) stats.trigger=i;
          }
     }
        
   DBG(poppop[poppopIndex]=micros()-start);
   DBG(poppopIndex=(poppopIndex+1)&15);
   return count;
}

#if 0
    #define DBG2(x) x;
#else
    #define DBG2(x)
#endif

DBG2(uint32_t poppop[16])
DBG2(uint32_t poppopIndex=0)

//  STM32 @ 128 M : Native      : 867 us
//  STM32 @ 128 M : reorganize  : 465 us
//                  trigger       450 us        

static int transformDma(int dc0_ac1,int16_t *in, float *out,int count, int expand,CaptureStats &stats, float triggerValue, DSOADC::TriggerMode mode,int swing)
{    
   if(!count) return false;
   if(expand==4096)
       return transformDmaExact(dc0_ac1,in,out,count,stats,triggerValue,mode,swing);
   DBG2(uint32_t start=micros());   
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
   
   // search min/max, take all the samples
   checkAvgMinMax(count,in,stats,swing,offset,multiplier);
 
   {   
    float f;
    for(int i=0;i<ocount;i++)
    {
        
        f=*(in+(dex/4096));
        f=QSUB(f,offset);
        f=QMUL(f,multiplier);
        out[i]=f; // Unit is now in volt
        dex+=expand;
    }   
   }
   // Search for trigger
     // Search for trigger
     if(stats.trigger==-1)
     {
         float t=triggerValue;
         t=t/multiplier;
         t+=offset;
         
         int tint=(int)t; // Check it in integer, faster
          if(mode!=DSOADC::Trigger_Rising)
          {
               for(int i=1;i<ocount && stats.trigger==-1;i++)
                    if(in[i-1]<tint&&in[i]>=tint) stats.trigger=i;
          }
         // The else is ***NOT** Missing here
          if(mode!=DSOADC::Trigger_Falling)
          {
               for(int i=1;i<ocount && stats.trigger==-1;i++)
                   if(in[i-1]>tint&&in[i]<=tint) stats.trigger=i;
          }
     }
   // The trigger needs to be compensated for expand
   if(stats.trigger!=-1)
   {
       float f=stats.trigger;
       f*=expand;
       f/=4096.;
       stats.trigger=(int)f;
   }
   
   DBG2(poppop[poppopIndex]=micros()-start);
   DBG2(poppopIndex=(poppopIndex+1)&15);
   return ocount;
}
static int transformDma2(int dc0_ac1,int16_t *in, float *out,int count, int expand,CaptureStats &stats, float triggerValue, DSOADC::TriggerMode mode,int swing)
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
       f=QSUB(f,offset);
       f=QMUL(f,multiplier);
       if(f>stats.xmax) stats.xmax=f;
       if(f<stats.xmin) stats.xmin=f;       
       out[0]=f; // Unit is now in volt
       stats.avg=QADD(stats.avg,f);
       dex+=expand;
   }
   
   // med
   //if(stats.trigger==-1)
   {   
    for(int i=1;i<ocount;i++)
    {

        f=*(in+(dex/4096));
        f=QSUB(f,offset);
        f=QMUL(f,multiplier);
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

        stats.avg=QADD(stats.avg,f);
        dex+=expand;
    }   
   }
   stats.avg/=(float)ocount;
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
    switch(set->dual)
    {
    case DSOADC::ADC_CAPTURE_FAST_INTERLEAVED:  return adc->prepareFastDualDMASampling(DSO_INPUT_PIN,set->rate,set->prescaler); break;
    case DSOADC::ADC_CAPTURE_MODE_NORMAL:  return adc->prepareDMASampling(set->rate,set->prescaler);break;
    default: xAssert(0);break;
    }
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
    
    // ask for  ~ the full buffer
    lastRequested=ADC_INTERNAL_BUFFER_SIZE-2;
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
    int     expand=tSettings[currentTime].expand4096;
    int needed=(expand*lastAskedSampleCount)/4096;
    int window=0;
    float *data=set->data;    
    p=((int16_t *)fset.set1.data);
    
    if(IS_CAPTURE_DUAL() )
        swapADCs(fset.set1.samples,(uint16_t *)p);
    
    if(trigger)
    {
        bool triggerFound=refineCapture(fset,needed);
        if(!triggerFound)
        {
            nextCapture();                
            return false;
        }        
    }
    set->stats.trigger=120; // right in the middle
    p=((int16_t *)fset.set1.data);
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
/**
 * 
 * @return 
 */
bool          DSOCapturePriv::initOnceDmaRunning()
{
    adc->setupDmaSampling();
    return true;
}
/**
 * 
 * @return 
 */
bool          DSOCapturePriv::initOnceDmaTrigger()
{
    adc->setupDmaSampling();
    return true;
}


int transformDmaExact2(int dc0_ac1,int16_t *in, float *out,int count, CaptureStats &stats, float triggerValue, DSOADC::TriggerMode mode,int swing)
{    
   if(!count) return false;
   DBG(uint32_t start=micros());
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
        f=QSUB(f,offset);
        f=QMUL(f,multiplier);
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

        stats.avg=QADD(stats.avg,f);
        dex++;
    }   
   }
   stats.avg/=(float)count;
   DBG(poppop[poppopIndex]=micros()-start);
   DBG(poppopIndex=(poppopIndex+1)&15);
   return count;
}

// EOF
