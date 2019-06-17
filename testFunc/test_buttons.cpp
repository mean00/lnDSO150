/***************************************************
 STM32 duino based firmware for DSO SHELL/150
 *  * GPL v2
 * (c) mean 2019 fixounet@free.fr
 ****************************************************/

#include "dso_includes.h"
#include "dso_test_signal.h"
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
    tft->setCursor(200, 5);
    tft->print("Rotary");
    tft->setCursor(200, 120);
    tft->print("Coupl");    

    while(1)
    {       
        controlButtons->updateCouplingState();
        //splash();
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
                xDelay(1000);
                Serial.println("Long");
            }
        }
        reCounter+=controlButtons->getRotaryValue();
        tft->setCursor(200, 30);
        tft->print(reCounter);
        tft->setCursor(200, 60);
        tft->print(ints);        
        
        tft->setCursor(200, 180);        
        tft->print(        controlButtons->geCouplingStateAsText());
        int raw=controlButtons->getRawCoupling();
        tft->setCursor(200, 150);
        tft->print( raw);
        
        //xDelay(30);         
    }
}
#define COUPLING_PIN PA5
//--
void testButtonCoupling(void)
{
  
    pinMode(COUPLING_PIN,INPUT_ANALOG);
    while(1)
    {   
            int rawCoupling= adc_read(&adc2, 5);            
            tft->setCursor(200, 150);
            tft->print( rawCoupling);
            xDelay(30);      
            tft->fillScreen(0);
    }
}

//-
