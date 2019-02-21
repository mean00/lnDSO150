
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



/**
 * \fn transform
 * \brief transform the raw input into scaled real value output
 * @param bfer
 * @param count
 * @param offset
 * @param voltageSCale
 * @return 
 */
int transform(int32_t *bfer, float *out,int count, VoltageSettings *set,int expand,float &xmin,float &xmax,float &avg)
{
   if(!count) return false;
   xmin=200;
   xmax=-200;
   avg=0;
   int ocount=(count*4096)/expand;
   ocount&=0xffe;
   int dex=0;
   for(int i=0;i<ocount;i++)
   {
       float f=bfer[dex/4096]>>16;
       f-=set->offset;
       f*=set->multiplier;
       if(f>xmax) xmax=f;
       if(f<xmin) xmin=f;       
       out[i]=f; // Unit is now in volt
       avg+=f;
       dex+=expand;
   }
   avg/=count;
   return ocount;
}