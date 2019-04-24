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

#include "dso_global.h"
#include "dso_display.h"
#include "dso_adc.h"


extern void splash(void);
static void drawGrid(void);

//--
extern Adafruit_TFTLCD_8bit_STM32 *tft;
extern DSOControl *controlButtons;
extern testSignal *myTestSignal;
//

static uint8_t waveForm[256]; // take a bit more, we have rounding issues
static bool voltageMode=false;


extern DSOADC *adc;

/**
 * 
 */
void testTrigger(void)
{


    DSOCapture::setTimeBase(    DSOCapture::DSO_TIME_BASE_5MS);
    DSOCapture::setVoltageRange(DSOCapture::DSO_VOLTAGE_5V);
    adc->setTriggerMode(DSOADC::Trigger_Both);
    int ratio=0;
    adc->setVrefPWM(ratio); 
    while(1)
    {        
        tft->fillScreen(0);
        bool trigger=adc->getTriggerState();
        tft->setCursor(10, 10);
        tft->print("Trigger");        
        tft->setCursor(10, 80);
        tft->print(trigger);
        tft->setCursor(10, 40);
        tft->print(ratio);
        if(!trigger)
        {
           // while(1)
            {
                  tft->setCursor(10, 60);
                  tft->print("Match"); 
            }
        }
        ratio+=1000;
        ratio&=0xFFFF;
        adc->setVrefPWM(ratio); 
        xDelay(50);
    }
        
} 


// EOF    
