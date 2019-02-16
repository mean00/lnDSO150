

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
extern DSOADC    *adc;
/**
 * 
 * @param array
 */

void header(int color,const char *txt)
{
    tft->fillScreen(BLACK);  
    tft->setTextSize(2);
    tft->setTextColor(color,BLACK);
    tft->setCursor(05, 40);
    tft->print(txt);
    tft->setCursor(05, 60);
    tft->print(" And press OK ");        
    while(!(controlButtons->getButtonEvents(DSOControl::DSO_BUTTON_OK)&EVENT_SHORT_PRESS))
    {
            
    }
}

void doCalibrate(uint16_t *array,int color, const char *txt)
{
    header(color,txt);        
    
    for(int range=0;range<14;range++)
    {
        controlButtons->setInputGain(range);        
        adc->initiateSampling(64);
        int count;
        uint32_t *xsamples=adc->getSamples(count);
        int sum=0;
        for(int i=0;i<count;i++)
        {
            sum+= xsamples[i]>>16;
        }
        sum/=count;
        adc->reclaimSamples(xsamples);
        array[range]=sum;
    }
}

/**
 * 
 * @return 
 */
bool DSOCalibrate::calibrate()
{
    adc->setTimeScale(ADC_SMPR_1_5,ADC_PRE_PCLK2_DIV_2); // 10 us *1024 => 10 ms scan
    header(RED,"Connect probe to GND") ;        
    doCalibrate(calibrationDC,YELLOW,"Switch CPL to *DC*");       
    doCalibrate(calibrationAC,GREEN, "Switch CPL to *AC*");
    return true;        
}
