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

// PA7 is timer3 channel2

extern Adafruit_TFTLCD_8bit_STM32 *tft;
extern testSignal *myTestSignal;


extern void splash(void);
void setTestSignal(int fq,bool high)
{
    static uint32_t bh, bl;
            
        splash();
        myTestSignal->setFrequency(fq);
        myTestSignal->setAmplitute(high);
        tft->setCursor(20, 30);
        tft->println(fq);
        tft->setCursor(200, 30);
        tft->println(high);
        xDelay(2000);
        
}

/**
 * 
 * @param a
 */
void testTestSignal( void)
{
    tft->setTextSize(3);
    
    while(1)
    {
        setTestSignal(10000,true);
        setTestSignal(10000,false);

        setTestSignal(2000,true);
        setTestSignal(2000,false);
        setTestSignal(1000,true);
        setTestSignal(1000,false);
        setTestSignal(500,true);
        setTestSignal(500,false);

    }
        
}

//-
