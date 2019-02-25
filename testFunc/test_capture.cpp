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

extern void splash(void);

static void drawGrid(void);
static void updateTimeScale();
//--
extern Adafruit_TFTLCD_8bit_STM32 *tft;
extern DSOControl *controlButtons;
extern int ints;
extern testSignal *myTestSignal;
//
static float voltageScale;

//
int transform(int32_t *bfer, float *out,int count, VoltageSettings *set,int expand,float &xmin,float &xmax,float &avg);

 

static float samples[256];


/**
 * 
 */

static inline int fromSample(float v)
{
//    v*=vSettings[currentVSettings].displayGain;              //1 Vol / div       
    v+=120;
    if(v>239) v=239;
    if(v<0) v=0;
    v=239-v;
    return (int)v;
}

static uint8_t waveForm[240];

static bool voltageMode=false;

void testCapture(void)
{
    DSODisplay::init();
    DSODisplay::drawGrid();
    int reCounter=0;
    
    tft->setTextSize(2);
    myTestSignal->setFrequency(2000); // 20Khz

    
    float xmin,xmax,avg;
    
    while(1)
    {
        int markStart,markEnd;

        int count;
        int i=0;
        // Ask samples , taking expand into account
        
        capture->initiateSampling(240);
        uint32_t *xsamples=capture->getSamples(count);
        markStart=millis();
        /*
        int scale=vSettings[currentVSettings].inputGain;
        
        count=transform((int32_t *)xsamples,samples,count,vSettings+currentVSettings,expand,xmin,xmax,avg);
        acquisitionTime=convTime*/
        
        capture->reclaimSamples(xsamples);
#if 0               
        tft->setCursor(240, 100);
        tft->print(currentTSettings);
        tft->setCursor(240, 120);
        tft->print(currentDiv);
        tft->setCursor(240, 200);
        tft->print(acquisitionTime);
         
            
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

            
        for(int j=0;j<count;j++)
        {
             waveForm[j]=fromSample(samples[j]); // in volt             
        }
        DSODisplay::drawWaveForm(count,waveForm);
#endif        
        tft->setCursor(240, 20);
        markEnd=millis();
        tft->print(markEnd-markStart);      
        int inc=controlButtons->getRotaryValue();
#if 0
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

                int x=(int)currentTime;
                x+=inc;
                currentTime=(adc_smp_rate)x;
                if(currentTime>ADC_SMPR_239_5) currentTime=ADC_SMPR_239_5;
                if(currentTime<ADC_SMPR_1_5) currentTime=ADC_SMPR_1_5;
                adc->setTimeScale(currentTime,ADC_PRE_PCLK2_DIV_2); 

                currentTSettings+=inc;
                if(currentTSettings>5) currentTSettings=5;
                if(currentTSettings<0) currentTSettings=0;
                DSODisplay::drawGrid();
                updateTimeScale();
                
            }
#endif         
    }
} 


// EOF    
