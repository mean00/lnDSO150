
#include <Wire.h>
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_TFTLCD_8bit_STM32.h"
//#include "Fonts/digitLcd56.h"
#include "Fonts/waree12.h"
#include "MapleFreeRTOS1000.h"
#include "MapleFreeRTOS1000_pp.h"
#include "testSignal.h"
#include "dso_control.h"
#include "HardwareSerial.h"
#include "dso_adc.h"

#include "transform.h"

/**
 * \fn transform
 * \brief transform the raw input into scaled real value output
 * @param bfer
 * @param count
 * @param offset
 * @param voltageSCale
 * @return 
 */
int maxOcountSeeen=0;
int transform(int16_t *in, float *out,int count, VoltageSettings *set,int expand,CaptureStats &stats, float triggerValue, DSOADC::TriggerMode mode)
{
   if(!count) return false;
   stats.xmin=200;
   stats.xmax=-200;
   stats.avg=0;
   int ocount=(count*4096)/expand;
   if(ocount>240)
   {
       maxOcountSeeen=ocount;
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

        f=*(in+2*(dex/4096));
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