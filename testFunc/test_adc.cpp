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
extern void splash(void);
//--
extern Adafruit_TFTLCD_8bit_STM32 *tft;
extern DSOControl *controlButtons;
extern int ints;
/**
 * 
 */
void testAdc(void)
{
    int val[256];
    int reCounter=0;
    while(1)
    {
        for(int i=0;i<16;i++)
        {
            controlButtons->setInputGain(i);
            for(int j=0;j<256;j++)
            {
                val[j]=analogRead(PA0);
            }
            int min=4096,max=0;
            for(int j=0;j<256;j++)
            {
                if(val[j]<min) min=val[j];
                if(val[j]>max) max=val[j];
                        
            }
            splash();
            tft->setCursor(20, 30);
            tft->println(i);
            tft->setCursor(20, 60);
            tft->println(min);
            tft->setCursor(20, 90);
            tft->println(max);
            xDelay(3000);
        }
    }
}
//-
