

#include "Adafruit_GFX.h"
#include "Adafruit_TFTLCD_8bit_STM32.h"
#include "MapleFreeRTOS1000.h"
#include "MapleFreeRTOS1000_pp.h"
#include "dsoControl.h"
#include "dso_adc.h"
#include "dso_global.h"
#include "dso_calibrate.h"

//--
extern Adafruit_TFTLCD_8bit_STM32 *tft;
extern DSOControl *controlButtons;
/**
 * 
 * @param array
 */
void doCalibrate(uint16_t *array)
{
    for(int range=0;range<14;range++)
    {
        controlButtons->setInputGain(range);
        int sum=0;
        for(int i=0;i<16;i++)
        {
            sum+=analogRead(PA0);
            xDelay(10);
        }
        sum/=16;
        array[range]=sum;
    }
}

/**
 * 
 * @return 
 */
bool DSOCalibrate::calibrate()
{
        tft->fillScreen(BLACK);   
        
        tft->setTextColor(WHITE,BLACK);
        tft->setFontSize(Adafruit_TFTLCD_8bit_STM32::SmallFont);
        tft->setCursor(45, 10);
        tft->myDrawString("    DSO-STM32duino");        
        
        tft->setCursor(45, 40);
        tft->myDrawString(" Set Coupling to DC ");        
        tft->setCursor(45, 60);
        tft->myDrawString(" And press OK ");        
        
        while(!(controlButtons->getButtonEvents(DSOControl::DSO_BUTTON_OK)&EVENT_SHORT_PRESS))
        {
            
        }
        doCalibrate(calibrationDC);
        tft->fillScreen(BLACK);   
        tft->setTextColor(YELLOW,BLACK);
        tft->setCursor(45, 40);
        tft->myDrawString(" Set Coupling to AC ");        
        tft->setCursor(45, 60);
        tft->myDrawString(" And press OK ");        
        while(!(controlButtons->getButtonEvents(DSOControl::DSO_BUTTON_OK)&EVENT_SHORT_PRESS))
        {
            
        }
        doCalibrate(calibrationAC);
        return true;        
}
