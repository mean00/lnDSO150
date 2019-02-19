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
//--
extern Adafruit_TFTLCD_8bit_STM32 *tft;
extern DSOControl *controlButtons;
extern int ints;
extern DSOADC    *adc;
extern testSignal *myTestSignal;
//
float voltageScale;
extern VoltageSettings vSettings[11] ;

//
bool transform(int32_t *bfer, float *out,int count, VoltageSettings *set,float &xmin,float &xmax,float &avg);

 

float samples[256];
int currentVSettings=5;

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


bool first=true;
void testAdc2(void)
{
    DSODisplay::init();
    DSODisplay::drawGrid();
    int reCounter=0;
    currentVSettings=7;
     controlButtons->setInputGain( vSettings[currentVSettings].inputGain); // x1.4
    tft->setTextSize(2);
    myTestSignal->setFrequency(20*1000); // 20Khz
    float xmin,xmax,avg;
    while(1)
    {
        int markStart,markEnd;

        int count;
        int i=0;
        adc->initiateSampling(240);
        uint32_t *xsamples=adc->getSamples(count);
        markStart=millis();
        int scale=vSettings[currentVSettings].inputGain;
        transform((int32_t *)xsamples,samples,count,vSettings+currentVSettings,xmin,xmax,avg);
        adc->reclaimSamples(xsamples);
            
#if 0            
            
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
        {
            currentVSettings+=inc;
            if(currentVSettings<0) currentVSettings=10;
            if(currentVSettings>10) currentVSettings=currentVSettings-10;
            DSODisplay::drawGrid();
            controlButtons->setInputGain( vSettings[currentVSettings].inputGain); // x1.4
        }
    }
} 

// EOF    
