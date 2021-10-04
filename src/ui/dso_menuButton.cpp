/***************************************************
 STM32 duino based firmware for DSO SHELL/150
 *  * GPL v2
 * (c) mean 2019 fixounet@free.fr
 ****************************************************/

#include "dso_menuEngine.h"
#include "dso_global.h"
#include "dso_gfx.h"
static int rotary=0;
static void header()
{
    DSO_GFX::clear(BLACK);
    DSO_GFX::setTextColor(BLACK,GREEN);    
    DSO_GFX::printxy(100, 0," Button Test ");
    DSO_GFX::setTextColor(GREEN,BLACK);
    DSO_GFX::printxy(40, 24 , " Long press OK to exit ");
   // DSO_GFX::printxy(100, 44,rotary);    
}

static void display(const char *title, const char *button, int i)
{
    header();
    DSO_GFX::printxy(20, 20+24*i,title);
    DSO_GFX::printxy(120, 20+24*i,button);
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
#if 0        
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
#endif        
    }
}

//-
