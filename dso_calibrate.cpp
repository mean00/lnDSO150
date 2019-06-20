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
 */
static void waitOk()
{
    while(!(controlButtons->getButtonEvents(DSOControl::DSO_BUTTON_OK)&EVENT_SHORT_PRESS)) {}
}

/**
 * 
 * @param array
 */
static void printxy(int x, int y, const char *t)
{
    tft->setCursor(x, y);
    tft->myDrawString(t);
}
/**
  */
static void printCalibrationTemplate( const char *st1, const char *st2)
{
    tft->fillScreen(BLACK);  
    printxy(0,10,"======CALIBRATION========");
    printxy(40,100,st1);
    printxy(8,120,st2);
    printxy(80,200,"and press OK");
}

/**
 * 
 * @param cpl
 */
static void printCoupling(DSOControl::DSOCoupling cpl)
{
    static const char *coupling[3]={"current : GND","current : DC ","current : AC "};
    printxy(40,130,coupling[cpl]);
      
}

void header(int color,const char *txt,DSOControl::DSOCoupling target)
{
    printCalibrationTemplate(txt,"");
    DSOControl::DSOCoupling   cpl=(DSOControl::DSOCoupling)-1;
    printxy(220,40," switch /\\");
    while(1)
    {
            DSOControl::DSOCoupling   newcpl=controlButtons->getCouplingState(); 
            if(newcpl==target) 
                tft->setTextColor(GREEN,BLACK);
            else  
                tft->setTextColor(RED,BLACK);

            if(cpl!=newcpl)
            {
                printCoupling(newcpl);
                cpl=newcpl;
            }
            if(cpl==target && controlButtons->getButtonEvents(DSOControl::DSO_BUTTON_OK)&EVENT_SHORT_PRESS)
            {
                tft->setTextColor(WHITE,BLACK);
                return;
            }
    }
}

void doCalibrate(uint16_t *array,int color, const char *txt,DSOControl::DSOCoupling target)
{
    
    printCalibrationTemplate("Connect probe to ground","(connect the 2 crocs together)");
    header(color,txt,target); 

    FullSampleSet fset;    
    uint32_t data [256];
    
    for(int range=0;range<14;range++)
    {
        controlButtons->setInputGain(range);        

        while(1)
        {
            adc->prepareDMASampling(ADC_SMPR_55_5,ADC_PRE_PCLK2_DIV_2);
            adc->startDMASampling(64);

            if(adc->getSamples(fset)) break;
        }
        uint16_t *xsamples=fset.set1.data;
        int sum=0;
        for(int i=0;i<fset.set1.samples;i++)
        {
            sum+= xsamples[i]>>16;
        }
        sum/=fset.set1.samples;
        array[range]=sum;
    }
}

/**
 * 
 * @return 
 */
bool DSOCalibrate::calibrate()
{
    
    tft->setFontSize(Adafruit_TFTLCD_8bit_STM32::MediumFont);  
    tft->setTextColor(WHITE,BLACK);
    
    
    
    adc->setTimeScale(ADC_SMPR_1_5,ADC_PRE_PCLK2_DIV_2); // 10 us *1024 => 10 ms scan
    printCalibrationTemplate("Connect probe to ground","(connect the 2 crocs together)");
    waitOk();
              
    doCalibrate(calibrationDC,YELLOW,"Set switch to *DC*",DSOControl::DSO_COUPLING_DC);       
    doCalibrate(calibrationAC,GREEN, "Set switch to *AC*",DSOControl::DSO_COUPLING_AC);
    return true;        
}
