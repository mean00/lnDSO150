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
//--
extern Adafruit_TFTLCD_8bit_STM32 *tft;
extern DSOControl *controlButtons;
extern int ints;
extern DSOADC    *adc;
/**
 * 
 */
void testAdc2(void)
{
    int reCounter=0;
    controlButtons->setInputGain(7); // x1.4
    
    uint16_t samples[256];
    
    int currentScale=10;
    adc->setTimeScale(currentScale); // 10 us *1024 => 10 ms scan
    while(1)
    {
        splash();
        tft->setCursor(200, 160);
        tft->print(currentScale);

      //  for(int i=0;i<16;i++)
        {
            int count;
            int i=0;
            adc->takeSamples(256);
            uint32_t *xsamples=adc->getSamples(count);
            for(int j=0;j<count;j++)
            {
                samples[j]=xsamples[j] >>16; //&0xffff;
            }

            int min=4095,max=0;
            for(int j=0;j<count;j++)
            {
                if(samples[j]<min) min=samples[j];
                if(samples[j]>max) max=samples[j];

            }
                        
            for(int j=0;j<count;j++)
            {
                float f=samples[j]; // 00--4096
                f/=20; // 0..200
                tft->drawPixel(16*i+j,240-f,YELLOW);
            }
        }
        xDelay(300);
        int inc=controlButtons->getRotaryValue();
        if(inc)
        {
            currentScale+=inc;
            if(currentScale<0) currentScale=0;
            currentScale&=0xf;
             controlButtons->setInputGain(currentScale);;
        }
        if(controlButtons->getButtonEvents(DSOControl::DSO_BUTTON_TIME)&EVENT_SHORT_PRESS)
        {            
            adc->setTimeScale(currentScale); 
        }
    }
}
//-
