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
    tft->setCursor(200, 1200);
    tft->print("Ints");    
    tft->setCursor(200, 190);
    tft->print("Coupling");    

    while(1)
    {        
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
        tft->setCursor(200, 160);
        tft->print(ints);
        
        tft->setCursor(200, 210);
        tft->print(        controlButtons->getCouplingState());
        
        //xDelay(30);         
    }
}
//-
