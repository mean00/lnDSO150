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
void testButtons(void)
{
    int reCounter=0;
    while(1)
    {
        tft->fillScreen(BLACK);   
        splash();
        for(int i=0;i<8;i++)
        {
            int evt=controlButtons->getButtonEvents((DSOControl::DSOButton)i);
            tft->setCursor(20, 30+i*20);
            tft->print(i);
            //Serial.print(i);
            if(evt & EVENT_SHORT_PRESS)
            {
                tft->print(" SHORT");
                Serial.println("Short");
                
            }
            if(evt & EVENT_LONG_PRESS)
            {
                tft->print(" LONG");
                Serial.println("Long");
            }
        }
        reCounter+=controlButtons->getRotaryValue();
        tft->setCursor(200, 30);
        tft->print(reCounter);
        tft->setCursor(200, 90);
        tft->setCursor(200, 160);
        tft->print(ints);
        xDelay(100);         
    }
}
//-
