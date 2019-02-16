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
//
bool transform(int32_t *bfer, float *out,int count, int offset, float voltageScale,float &xmin,float &xmax);

float inputScale[16]={0,1./14.,1./7.,1./3.5,
                     1./1.4,1./0.7,1./0.35,7,
                     14,29,71,143,
                     286,286,286};

int calibration[16]={1893,1868,1880,1890,
                    1896,1887,1891,1904,
                    1790,1788,1786,1887,1896};

float samples[256];
void updateCurrentVoltageScale(int scale);
/**
 * 
 */
void testAdc2(void)
{
    int reCounter=0;
    int currentScale=10;        
    controlButtons->setInputGain(7); // x1.4
    currentScale=7;
    updateCurrentVoltageScale(currentScale);
    adc->setTimeScale(ADC_SMPR_1_5,ADC_PRE_PCLK2_DIV_2); // 10 us *1024 => 10 ms scan
    myTestSignal->setFrequency(20*1000); // 20Khz
    float xmin,xmax;
    while(1)
    {
        int markStart,markEnd;
        {
            int count;
            int i=0;
            adc->initiateSampling(240);
            uint32_t *xsamples=adc->getSamples(count);
            markStart=millis();
            transform((int32_t *)xsamples,samples,count,calibration[currentScale],voltageScale,xmin,xmax);
            adc->reclaimSamples(xsamples);
            
            
            
            tft->setCursor(240, 100);
            tft->print((float)DSOADC::getVCCmv()/1000.);

            tft->setCursor(240, 120);
            tft->print(xmin);
            tft->setCursor(240, 140);
            tft->print(xmax);
            
            float last=samples[0]; // 00--4096
            last*=24;
            if(last>239) last=239;
            if(last<0) last=0;
            last=239-last;
            
            uint16_t colors[240];
            markStart=millis();
            for(int j=1;j<count;j++)
            {
                float next=samples[j]; // in volt
                next*=24;              //1 Vol / div
                
                if(next>239) next=239;
                if(next<0) next=0;
                
                next=239-next;
                if(next==last) last++;
                uint16_t color;
                if(!(j%24))
                    color=(0xF)<<5;
                else 
                    color=0;
                for(int i=0;i<240;i++) colors[i]=color;
                int start,end;
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
                for(int i=start;i<=end;i++) colors[i]=YELLOW;
                tft->setAddrWindow(j,0,j,240);
                tft->pushColors(colors,240,true);
                last=next;
            }
        }
        markEnd=millis();
        tft->setCursor(240, 20);
        tft->print(markEnd-markStart);
        xDelay(200); // 50 i/s
        int inc=controlButtons->getRotaryValue();
        if(inc)
        {
            currentScale+=inc;
            if(currentScale<0) currentScale=0;
            currentScale&=0xf;
             controlButtons->setInputGain(currentScale);;
             updateCurrentVoltageScale(currentScale);
        }
        if(controlButtons->getButtonEvents(DSOControl::DSO_BUTTON_TIME)&EVENT_SHORT_PRESS)
        {            
         //   adc->setTimeScale(currentScale); 
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
    float v=inputScale[scale];
    v*=(float)DSOADC::getVCCmv();
    v/=4096000.; 
    voltageScale=v; // output in V
    
}
