/***************************************************
 STM32 duino based firmware for DSO SHELL/150
 *  * GPL v2
 * (c) mean 2019 fixounet@free.fr
 ****************************************************/

#include "dso_includes.h"
#include "dso_adc_gain_priv.h"
#include "dso_test_signal.h"
extern void splash(void);

static void drawGrid(void);
//--
extern Adafruit_TFTLCD_8bit_STM32 *tft;
extern DSOControl *controlButtons;
extern int ints;
extern DSOADC    *adc;
extern testSignal *myTestSignal;
/**
 * 
 */



#define NB_SAMPLES 64
/**
 * 
 * @return 
 */
static int averageADCRead()
{
#if 0
    // Start Capture
    adc->prepareTimerSampling(500); // 1Khz
    adc->startTimerSampling(64);
    FullSampleSet fset;
    while(!adc->getSamples(fset))
    {
        
    };
    int nb=fset.set1.samples;
    int sum=0;
    for(int i=0;i<nb;i++)
    {
        sum+=fset.set1.data[i];
    }
    sum=(sum+(nb/2)-1)/nb;
    
    return sum;
#endif
}

/**
 * 
 * @param array
 * @param color
 * @param txt
 * @param target
 */
static void testDoCalibrate(uint16_t *array,int color, const char *txt,DSOControl::DSOCoupling target)
{

    for(int range=0;range<DSO_NB_GAIN_RANGES;range++)
    {
        DSOInputGain::setGainRange((DSOInputGain::InputGainRange) range);
        xDelay(10);
        array[range]=averageADCRead();
        tft->print(range);
        tft->print(":");
        tft->setCursor(50, 10+16*range);
        tft->print((int)calibrationDC[range]);
    }
}

void testCalibrate(void)
{
    int reCounter=0;
    tft->fillScreen(0);
    tft->setTextSize(2);
    
    adc->setTimeScale(ADC_SMPR_1_5,DSOADC::ADC_PRESCALER_2); // 10 us *1024 => 10 ms scan
    testDoCalibrate(calibrationDC,YELLOW,"",DSOControl::DSO_COUPLING_DC);       
    
    
    while(1)
    {
        
    }
    
}
