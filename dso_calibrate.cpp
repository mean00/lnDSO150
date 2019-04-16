/***************************************************
 STM32 duino based firmware for DSO SHELL/150
 *  * GPL v2
 * (c) mean 2019 fixounet@free.fr
 ****************************************************/

#include "dso_global.h"
#include "dso_adc.h"
extern DSOADC                     *adc;
/**
 * 
 * @param array
 */

static void printCoupling(DSOControl::DSOCoupling cpl)
{
    static const char *coupling[3]={"GND","DC ","AC "};
    tft->setCursor(100, 120);
    tft->print(coupling[cpl]);    
}

void header(int color,const char *txt,DSOControl::DSOCoupling target)
{
    tft->fillScreen(BLACK);  
    tft->setTextSize(2);
    tft->setTextColor(color,BLACK);
    tft->setCursor(05, 40);
    tft->print(txt);
    tft->setCursor(05, 60);
    tft->print(" And press OK ");    
    DSOControl::DSOCoupling   cpl=controlButtons->getCouplingState();  
    printCoupling(cpl);
    while(1)
    {
            DSOControl::DSOCoupling   newcpl=controlButtons->getCouplingState(); 
            if(cpl!=newcpl)
            {
                printCoupling(newcpl);
                cpl=newcpl;
            }
            if(controlButtons->getButtonEvents(DSOControl::DSO_BUTTON_OK)&EVENT_SHORT_PRESS)
            {
                if(cpl==target)
                    return;
            }
    }
}

void doCalibrate(uint16_t *array,int color, const char *txt,DSOControl::DSOCoupling target)
{
    header(color,txt,target); 
    
    
    SampleSet set;
    uint32_t data [256];
    
    set.data=data;
    
    for(int range=0;range<14;range++)
    {
        controlButtons->setInputGain(range);        

        adc->prepareDMASampling(ADC_SMPR_55_5,ADC_PRE_PCLK2_DIV_2);
        adc->startDMASampling(64);
        
        adc->getSamples(set);
        uint32_t *xsamples=set.data;
        int sum=0;
        for(int i=0;i<set.samples;i++)
        {
            sum+= xsamples[i]>>16;
        }
        sum/=set.samples;
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
    header(RED,"Connect probe to GND",DSOControl::DSO_COUPLING_GND) ;        
    doCalibrate(calibrationDC,YELLOW,"Switch CPL to *DC*",DSOControl::DSO_COUPLING_DC);       
    doCalibrate(calibrationAC,GREEN, "Switch CPL to *AC*",DSOControl::DSO_COUPLING_AC);
    return true;        
}
