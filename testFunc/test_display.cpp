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
#include "dsoControl.h"
#include "HardwareSerial.h"
#include "dso_adc.h"
extern void splash(void);

static void drawGrid(void);
static void drawRightBar();
//--
extern Adafruit_TFTLCD_8bit_STM32 *tft;
extern DSOControl *controlButtons;
extern int ints;
extern DSOADC    *adc;
/**
 * 
 */
void testDisplay(void)
{
        splash();
        drawGrid();
        drawRightBar();
        
        
        tft->setTextColor(WHITE,BLACK);
        tft->setFontSize(Adafruit_TFTLCD_8bit_STM32::SmallFont);
        tft->setCursor(242,10);
        tft->myDrawString("1000mv");        

        tft->setCursor(242,40);
        tft->myDrawString("10mv/D");        
        tft->setCursor(242,60);
        tft->myDrawString("100us/D");        
        
        
    while(1)
    {
       
        xDelay(300);
        int inc=controlButtons->getRotaryValue();
       
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

 void drawRightBar()
 {
     for(int i=240;i<320;i++)
        tft->drawFastVLine(i,0,240,WHITE);
 }