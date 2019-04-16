/***************************************************
 STM32 duino based firmware for DSO SHELL/150
 *  * GPL v2
 * (c) mean 2019 fixounet@free.fr
 ****************************************************/

#include <Wire.h>
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_TFTLCD_8bit_STM32.h"
#include "Fonts/Targ56.h"
#include "Fonts/digitLcd56.h"
#include "Fonts/FreeSansBold12pt7b.h"
#include "MapleFreeRTOS1000.h"
#include "MapleFreeRTOS1000_pp.h"
#include "testSignal.h"
#include "dso_control.h"
#include "HardwareSerial.h"
#include "dso_adc.h"
extern void splash(void);

static void drawGrid(void);
//--
extern Adafruit_TFTLCD_8bit_STM32 *tft;
extern DSOControl *controlButtons;
extern int ints;
extern DSOADC    *adc;
/**
 * 
 */

void testOne(adc_smp_rate one, adc_prescaler two,int sc)
{
#warning FIXME

   static uint32 before,after;
   
   before= micros();
   
   adc->prepareDMASampling (one,two);            
   adc->startDMASampling (1024);
   SampleSet    set;
   uint32_t data[256];
   set.data=data;
   //bool r=adc->getSamples(set);

    
   after= micros();
   //printf("%d:%d => %d\n",(int)one,sc,after-before);
   Serial.print("Rate:");
   Serial.print(one);
   Serial.print("Prescaler:");
   Serial.print(sc);
   Serial.print("Time(us):");
   Serial.println(after-before);

}

void testAdc(void)
{
    controlButtons->setInputGain(7); // x1.4
    while(1)
    {
        
        for(int i=ADC_SMPR_1_5;i<=ADC_SMPR_239_5;i++)
        {
            testOne((adc_smp_rate)i,ADC_PRE_PCLK2_DIV_2,2);
            testOne((adc_smp_rate)i,ADC_PRE_PCLK2_DIV_4,4);
            testOne((adc_smp_rate)i,ADC_PRE_PCLK2_DIV_6,6);
            testOne((adc_smp_rate)i,ADC_PRE_PCLK2_DIV_8,8);
        }
        xDelay(5000);
    }
  
}
//-
#define SCALE_STEP 24
#define C 10
void drawGrid(void)
{
    uint16_t fgColor=(0x1F)<<5;
    for(int i=0;i<=C;i++)
    {
        tft->drawFastHLine(0,SCALE_STEP*i,SCALE_STEP*C,fgColor);
        tft->drawFastVLine(SCALE_STEP*i,0,SCALE_STEP*C,fgColor);
    }
       tft->drawFastHLine(0,239,SCALE_STEP*C,fgColor);
}