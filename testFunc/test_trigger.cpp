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

#if 0
    DSOCapture::setTimeBase(    DSOCapture::DSO_TIME_BASE_5MS);
    DSOCapture::setVoltageRange(DSOCapture::DSO_VOLTAGE_5V);
    adc->setTriggerMode(DSOADC::Trigger_Both);
    int ratio=0;
    while(1)
    {        
        tft->fillScreen(0);
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
#endif        
} 


// EOF    
