/***************************************************
 STM32 duino based firmware for DSO SHELL/150
 *  * GPL v2
 * (c) mean 2019 fixounet@free.fr
 ****************************************************/

#include "dso_includes.h"

/**
 */
void  menuManagement(void)
{
     tft->fillScreen(BLACK);
     while(1)
     {
        if(controlButtons->getButtonEvents(DSOControl::DSO_BUTTON_OK) & EVENT_LONG_PRESS)    
        {
            return;
        }
     }
}