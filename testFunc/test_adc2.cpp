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
#include "pattern.h"
#include "dso_global.h"
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
void updateCurrentVoltageScale(int scale);
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

uint8_t prePos[240];
uint8_t preLength[240];

bool first=true;
void testAdc2(void)
{
    int reCounter=0;
    updateCurrentVoltageScale(7);
    tft->setTextSize(2);
    myTestSignal->setFrequency(20*1000); // 20Khz
    float xmin,xmax,avg;
    while(1)
    {
        int markStart,markEnd;
        {
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
            
            int last=fromSample(samples[0]);            
            
            
            for(int j=1;j<count;j++)
            {
                 int next=fromSample(samples[j]); // in volt
               
                int start,end;
                if(next==last)
                {
                    last++;
                }
                if(next>last)
                {
                    start=last;
                    end=next;
                }
                else
                {
                    start=next;
                    end=last;
                    
                }
                
                int seg1=start;
                int seg2=end-start;
                int seg3=239-end;

                uint16_t *bg=(uint16_t *)defaultPattern;
                if(!(j%24)) bg=(uint16_t *)darkGreenPattern;

                
                if(first)
                {
                    first=false;
                }else
                {
                    // cleanup prev draw
                    tft->setAddrWindow(j,prePos[j],j,240);
                    tft->pushColors(((uint16_t *)bg)+prePos[j],
                                        preLength[j],true);
                }
                
                tft->drawFastVLine(j,start,seg2,YELLOW);
                preLength[j]=seg2;
                prePos[j]=start;
                
                last=next;
            }
        }
        
        tft->setCursor(240, 20);
        markEnd=millis();
        tft->print(markEnd-markStart);
      //  xDelay(60); // 50 i/s
        int inc=controlButtons->getRotaryValue();
        if(inc)
        {
            currentVSettings+=inc;
            if(currentVSettings<0) currentVSettings=10;
            if(currentVSettings>10) currentVSettings=currentVSettings-10;
            tft->fillScreen(0);
            updateCurrentVoltageScale(currentVSettings);
        }
    }
} 
//-
#define SCALE_STEP 24
#define C 10
#define CENTER_CROSS 1
void drawGrid(void)
{
    uint16_t fgColor=(0xF)<<5;
    for(int i=0;i<=C;i++)
    {
        tft->drawFastHLine(0,SCALE_STEP*i,SCALE_STEP*C,fgColor);
        tft->drawFastVLine(SCALE_STEP*i,0,SCALE_STEP*C,fgColor);
    }
    tft->drawFastHLine(0,239,SCALE_STEP*C,fgColor);
    
    tft->drawFastHLine(SCALE_STEP*(C/2-CENTER_CROSS),SCALE_STEP*5,SCALE_STEP*CENTER_CROSS*2,WHITE);
    tft->drawFastVLine(SCALE_STEP*5,SCALE_STEP*(C/2-CENTER_CROSS),SCALE_STEP*CENTER_CROSS*2,WHITE);
}
    

void updateCurrentVoltageScale(int scale)
{
    currentVSettings=scale;
    controlButtons->setInputGain( vSettings[currentVSettings].inputGain); // x1.4
}
