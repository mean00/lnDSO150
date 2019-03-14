/***************************************************
 STM32 duino based firmware for DSO SHELL/150
 *  * GPL v2
 * (c) mean 2019 fixounet@free.fr
 ****************************************************/

#include <Wire.h>
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_TFTLCD_8bit_STM32.h"
#include "Fonts/Targ56.h"
#include "Fonts/digitLcd56.h"
#include "Fonts/FreeSansBold12pt7b.h"
#include "MapleFreeRTOS1000.h"
#include "MapleFreeRTOS1000_pp.h"
#include "testSignal.h"
#include "dsoControl.h"
#include "HardwareSerial.h"
#include "dso_adc.h"
#include "dso_global.h"
#include "dsoDisplay.h"
#include "transform.h"
extern void splash(void);


//--
extern Adafruit_TFTLCD_8bit_STM32 *tft;
extern DSOControl *controlButtons;
extern int ints;
extern DSOADC    *adc;
extern testSignal *myTestSignal;
//
static float voltageScale;
extern VoltageSettings vSettings[11] ;
static const TimeSettings tSettings[6]
{
    {"10us",    ADC_PRE_PCLK2_DIV_2,ADC_SMPR_1_5,   4390},
    {"25us",    ADC_PRE_PCLK2_DIV_2,ADC_SMPR_13_5,  5909},
    {"50us",    ADC_PRE_PCLK2_DIV_2,ADC_SMPR_55_5,  4496},
    {"100us",   ADC_PRE_PCLK2_DIV_4,ADC_SMPR_55_5,  4517},
    {"500us",   ADC_PRE_PCLK2_DIV_4,ADC_SMPR_239_5, 6095},
    {"1ms",     ADC_PRE_PCLK2_DIV_8,ADC_SMPR_239_5, 6095}
};



//


 

static float samples[256];
static int currentVSettings=5;
static int currentTSettings=0;

static void updateTimeScale();

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

static uint8_t waveForm[240];

static int currentDiv=3;
static bool voltageMode=true;
static uint32_t acquisitionTime;
extern uint32_t convTime;
static int expand;
static bool first=true;
void testAdc3(void)
{
    DSODisplay::init();
    DSODisplay::drawGrid();
    int reCounter=0;
    currentVSettings=8;
    controlButtons->setInputGain( vSettings[currentVSettings].inputGain); // x1.4
    tft->setTextSize(2);
    myTestSignal->setFrequency(50); // 100 hz /10 ms period
    updateTimeScale();
    
    float xmin,xmax,avg;
    expand=4096;
    while(1)
    {

        int markStart,markEnd;
        CaptureStats stats;
        int count;
        int i=0;
        // Ask samples , taking expand into account
        adc->prepareTimerSampling (1000);            
        adc->startTimerSampling ((240*expand)/4096);
        
        SampleSet    *set=adc->getSamples();
        count=set->samples;
        uint32_t *xsamples=set->data;
        
        markStart=millis();
        int scale=vSettings[currentVSettings].inputGain;
        
        count=transform((int32_t *)xsamples,samples,count,vSettings+currentVSettings,expand,stats);
        acquisitionTime=convTime/1000;
        adc->reclaimSamples(set);
            
        tft->setCursor(240, 100);
        tft->print(currentTSettings);
        tft->setCursor(240, 120);
        tft->print(currentDiv);
        tft->setCursor(240, 200);
        tft->print(acquisitionTime);
#if 1            
            
            tft->setCursor(240, 100);
            tft->print((float)DSOADC::getVCCmv()/1000.);
            tft->setCursor(240, 120);
            tft->print(xmin);
            tft->setCursor(240, 140);
            tft->print(xmax);            
            tft->setCursor(240, 160);
            tft->print(avg);            
            
            tft->setCursor(240, 60);
            tft->print(scale);

            tft->setCursor(240, 40);
            tft->print(vSettings[currentVSettings].name);
#endif
            
        for(int j=0;j<count;j++)
        {
             waveForm[j]=fromSample(samples[j]); // in volt             
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
} 

void updateTimeScale()
{
    adc->setTimeScale(tSettings[currentTSettings].rate,tSettings[currentTSettings].prescaler);     
    expand=tSettings[currentTSettings].expand4096;
    tft->setCursor(240, 50);
    tft->print(tSettings[currentTSettings].name);
}

// EOF    