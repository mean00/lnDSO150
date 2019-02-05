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
#define NB_SAMPLES 512
    uint16_t val[NB_SAMPLES+1];
    int reCounter=0;
    controlButtons->setInputGain(8); // Full blast
    
#if 0    
    while(1)
    {
        splash();
        int before=millis();
        for(int i=0;i<10000;i++)
        {
            analogRead(PA0);
        }
        int after=millis();
        tft->setCursor(20, 30);
        tft->println(before);
        
        tft->setCursor(20, 90);
        tft->println(after);
        
        tft->setCursor(20, 130);
        tft->println(after-before);

        
        xDelay(2000);
    }
    
#endif    
      pinMode(PA0,INPUT_ANALOG);
    while(1)
    {
        for(int i=0;i<16;i++)
        {
         //   controlButtons->setInputGain(i);
            
            
            for(int j=0;j<NB_SAMPLES;j++)
            {
                val[j]=analogRead(PA0);
                vTaskDelay(portTICK_PERIOD_MS/10);
            }
            int min=4096,max=0;
            for(int j=0;j<NB_SAMPLES;j++)
            {
                if(val[j]<min) min=val[j];
                if(val[j]>max) max=val[j];
                        
            }
            splash();
            for(int j=0;j<NB_SAMPLES;j++)
            {
                float f=val[j*(NB_SAMPLES)/256]; // 00--4096
                f/=20; // 0..200
                tft->drawPixel(j,240-f,YELLOW);
                
                
            }
            tft->setCursor(20, 30);
            tft->println(i);
            tft->setCursor(20, 60);
            tft->println(min);
            tft->setCursor(20, 90);
            tft->println(max);
            //xDelay(3000);
        }
    }
}
//-
