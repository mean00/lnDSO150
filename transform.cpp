
#include <Wire.h>
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_TFTLCD_8bit_STM32.h"
//#include "Fonts/digitLcd56.h"
#include "Fonts/waree12.h"
#include "MapleFreeRTOS1000.h"
#include "MapleFreeRTOS1000_pp.h"
#include "testSignal.h"
#include "dsoControl.h"
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
int transform(int32_t *bfer, float *out,int count, VoltageSettings *set,int expand,CaptureStats &stats)
{
   if(!count) return false;
   stats.xmin=200;
   stats.xmax=-200;
   stats.avg=0;
   int ocount=(count*4096)/expand;
   ocount&=0xffe;
   int dex=0;
   for(int i=0;i<ocount;i++)
   {
       float f=bfer[dex/4096]>>16;
       f-=set->offset;
       f*=set->multiplier;
       if(f>stats.xmax) stats.xmax=f;
       if(f<stats.xmin) stats.xmin=f;       
       out[i]=f; // Unit is now in volt
       stats.avg+=f;
       dex+=expand;
   }
   stats.avg/=count;
   return ocount;
}