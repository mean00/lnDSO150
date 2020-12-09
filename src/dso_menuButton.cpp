/***************************************************
 STM32 duino based firmware for DSO SHELL/150
 *  * GPL v2
 * (c) mean 2019 fixounet@free.fr
 ****************************************************/

#include "dso_includes.h"
#include "dso_menuEngine.h"
#include "dso_global.h"
static int rotary=0;
static void header()
{
    tft->fillScreen(BLACK);
    tft->setTextColor(BLACK,GREEN);
    tft->setCursor(100, 0);        
    tft->print(" Button Test ");
    tft->setTextColor(GREEN,BLACK);
    tft->setCursor(40, 24);        
    tft->print(" Long press OK to exit ");
    
    tft->setCursor(100, 44);        
    tft->print(rotary);
    
}

static void display(const char *title, const char *button, int i)
{
    header();
    tft->setCursor(20, 20+24*i);        
    tft->print(title);
    tft->setCursor(120, 20+24*i);        
    tft->print(button);
    xDelay(1000);
    header();    
}

/**
 * 
 */
void buttonTest(void)
{
    header();
    
    while(1)
    {       
        xDelay(50);
        int inc=controlButtons->getRotaryValue();
        if(inc)
        {
            rotary+=inc;
            header();
        }
        //splash();
        for(int i=0;i<8;i++)
        {
            DSOControl::DSOButton button=(DSOControl::DSOButton)i;
            int evt=controlButtons->getButtonEvents(button);
            
            if(evt & EVENT_SHORT_PRESS)
            {
                display("Short",DSOControl::getName(button),i);
            }
            if(evt & EVENT_LONG_PRESS)
            {
                display("Long",DSOControl::getName(button),i);
                if(button==DSOControl::DSO_BUTTON_OK)
                    return;
            }
            
        }
    }
}

//-
