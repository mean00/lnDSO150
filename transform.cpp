
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
bool transform(int32_t *bfer, float *out,int count, int offset, float voltageScale,float &xmin,float &xmax)
{
   xmin=200;
   xmax=-200;
   for(int i=0;i<count;i++)
   {
       float f=bfer[i]>>16;
       f-=offset;
       f*=voltageScale;
       if(f>xmax) xmax=f;
       if(f<xmin) xmin=f;       
       out[i]=f; // Unit is now *1024, avoid using float later on
   }
   return true;
}