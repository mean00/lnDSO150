/***************************************************
 STM32 duino based firmware for DSO SHELL/150
 *  * GPL v2
 * (c) mean 2019 fixounet@free.fr
 ****************************************************/

#include "dso_includes.h"
#include "dso_test_signal.h"

// PA7 is timer3 channel2

extern Adafruit_TFTLCD_8bit_STM32 *tft;
extern testSignal *myTestSignal;


extern void splash(void);
void setTestSignal(int fq,bool high)
{
    static uint32_t bh, bl;
            
        splash();
        myTestSignal->setFrequency(fq);
        myTestSignal->setAmplitude(high);
        tft->setCursor(20, 30);
        tft->println(fq);
        tft->setCursor(200, 30);
        tft->println(high);
        xDelay(6000);
        
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
