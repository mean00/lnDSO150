/***************************************************
 STM32 duino based firmware for DSO SHELL/150
 *  * GPL v2
 * (c) mean 2019 fixounet@free.fr
 ****************************************************/

#include "dso_includes.h"
#include "dso_test_signal.h"
#include "dso_adc.h"

extern void splash(void);

static void drawGrid(void);
//--
extern Adafruit_TFTLCD_8bit_STM32 *tft;
extern DSOControl *controlButtons;
extern int ints;
extern DSOADC    *adc;
extern testSignal *myTestSignal;
//
float voltageScale;
static const TimeSettings tSettings[6]
{
    {DSOADC::ADC_CAPTURE_MODE_NORMAL,"10us",    DSOADC::ADC_PRESCALER_2  ,ADC_SMPR_1_5,   4390},
    {DSOADC::ADC_CAPTURE_MODE_NORMAL,"25us",    DSOADC::ADC_PRESCALER_2,  ADC_SMPR_13_5,  5909},
    {DSOADC::ADC_CAPTURE_MODE_NORMAL,"50us",    DSOADC::ADC_PRESCALER_2,  ADC_SMPR_55_5,  4496},
    {DSOADC::ADC_CAPTURE_MODE_NORMAL,"100us",   DSOADC::ADC_PRESCALER_4,  ADC_SMPR_55_5,  4517},
    {DSOADC::ADC_CAPTURE_MODE_NORMAL,"500us",   DSOADC::ADC_PRESCALER_8,  ADC_SMPR_239_5, 6095},
    {DSOADC::ADC_CAPTURE_MODE_NORMAL,"1ms",     DSOADC::ADC_PRESCALER_8,  ADC_SMPR_239_5, 6095}
};

//

 

float test_samples[256];
int currentVSettings=5;
int currentTSettings=0;

void updateTimeScale();

/**
 * 
 */

static inline int fromSample(float v)
{
    v*=vSettings[currentVSettings].displayGain;              //1 Vol / div       
    v+=120;
    if(v>239) v=239;
    if(v<0) v=0;
    v=239-v;
    return (int)v;
}

uint8_t waveForm[240];

int currentDiv=3;
bool voltageMode=false;
uint32_t acquisitionTime;

int expand;
bool first=true;
void testAdc2(void)
{
#if 0
    DSODisplay::init();
    DSODisplay::drawGrid();
    int reCounter=0;
    currentVSettings=7;
    controlButtons->setInputGain( vSettings[currentVSettings].inputGain); // x1.4
    tft->setTextSize(2);
    myTestSignal->setFrequency(2000); // 2Khz
    updateTimeScale();
    
    while(1)
    {
        int markStart,markEnd;
        CaptureStats stats;

        int count;
        int i=0;
        // Ask samples , taking expand into account
        
        adc->prepareDMASampling (tSettings[currentTSettings].rate,tSettings[currentTSettings].prescaler);            
        adc->startDMASampling ((240*expand)/4096);
        
        FullSampleSet  s;
        bool r=adc->getSamples(s);
        count=s.set1.samples;
        int16_t *xsamples=(int16_t* )s.set1.data;
        markStart=millis();
        int scale=vSettings[currentVSettings].inputGain;
        
        count=transformTimer(xsamples,(float *)test_samples,count,vSettings+currentVSettings,expand,stats,1.0,DSOADC::Trigger_Both);
        acquisitionTime=convTime;
//        adc->reclaimSamples(set);
            
        tft->setCursor(240, 100);
        tft->print(currentTSettings);
        tft->setCursor(240, 120);
        tft->print(currentDiv);
        tft->setCursor(240, 200);
        tft->print(acquisitionTime);
       
            
        tft->setCursor(240, 100);
        tft->print((float)DSOADC::getVCCmv()/1000.);
        tft->setCursor(240, 120);
        tft->print(stats.xmin);
        tft->setCursor(240, 140);
        tft->print(stats.xmax);            
        tft->setCursor(240, 160);
        tft->print(stats.avg);            

        tft->setCursor(240, 60);
        tft->print(scale);

        tft->setCursor(240, 40);
        tft->print(vSettings[currentVSettings].name);
            
        for(int j=0;j<count;j++)
        {
             waveForm[j]=fromSample(test_samples[j]); // in volt             
        }
        DSODisplay::drawWaveForm(count,waveForm);
        
        tft->setCursor(240, 20);
        markEnd=millis();
        tft->print(markEnd-markStart);      
        int inc=controlButtons->getRotaryValue();
        if(inc)
            if(voltageMode)
            {
                currentVSettings+=inc;
                if(currentVSettings<0) currentVSettings=10;
                if(currentVSettings>10) currentVSettings=currentVSettings-10;
                DSODisplay::drawGrid();
                controlButtons->setInputGain( vSettings[currentVSettings].inputGain); // x1.4
            }
            else
            {
#if 0
                int x=(int)currentTime;
                x+=inc;
                currentTime=(adc_smp_rate)x;
                if(currentTime>ADC_SMPR_239_5) currentTime=ADC_SMPR_239_5;
                if(currentTime<ADC_SMPR_1_5) currentTime=ADC_SMPR_1_5;
                adc->setTimeScale(currentTime,ADC_PRE_PCLK2_DIV_2); 
#endif
                currentTSettings+=inc;
                if(currentTSettings>5) currentTSettings=5;
                if(currentTSettings<0) currentTSettings=0;
                DSODisplay::drawGrid();
                updateTimeScale();
                
            }
         
    }
#endif
} 

void updateTimeScale()
{
    
    expand=tSettings[currentTSettings].expand4096;
    tft->setCursor(240, 50);
    tft->print(tSettings[currentTSettings].name);
}

// EOF    
