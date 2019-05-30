/***************************************************
 STM32 duino based firmware for DSO SHELL/150
 *  * GPL v2
 * (c) mean 2019 fixounet@free.fr
 ****************************************************/

#include "dso_includes.h"
#include "dso_test_signal.h"
extern void splash(void);

static void drawGrid(void);
//--
extern Adafruit_TFTLCD_8bit_STM32 *tft;
extern DSOControl *controlButtons;
extern int ints;
extern DSOADC    *adc;
extern testSignal *myTestSignal;
/**
 * 
 */
void testCalibrate(void)
{
    int reCounter=0;
    tft->fillScreen(0);
    tft->setTextSize(2);
    for(int i=0;i<14;i++)
    {
         controlButtons->setInputGain(i);
         xDelay(100);
         float v=0;
         for(int i=0;i<16;i++)
         {
             v+=analogRead(analogInPin);;
             delay(1);
         }
         v/=16.;         
        tft->setCursor(10, 10+16*i);
        tft->print(i);
        tft->setCursor(50, 10+16*i);
        tft->print((int)v);
    }
   
    
}
