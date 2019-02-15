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
extern bool transform(int32_t *bfer, int count, int offset, float voltageScale,float &xmin,float &xmax);

float inputScale[16]={0,1./14.,1./7.,1./3.5,
                     1./1.4,1./0.7,1./0.35,7,
                     14,29,71,143,
                     286,286,286};

void updateCurrentVoltageScale(int scale);
/**
 * 
 */
void testAdc2(void)
{
    int reCounter=0;
    
    
    int32_t samples[256];
    
    int currentScale=10;
    
    // Calibrate...
    
    int cal=0;
    int avg;
    controlButtons->setInputGain(0);
    for(int i=0;i<256;i++)
    {
        cal+=analogRead(PA0);
        xDelay(2);
    }
    cal>>=8;
    
    
    controlButtons->setInputGain(7); // x1.4
    currentScale=7;
    updateCurrentVoltageScale(currentScale);
    adc->setTimeScale(ADC_SMPR_1_5,ADC_PRE_PCLK2_DIV_2); // 10 us *1024 => 10 ms scan
    myTestSignal->setFrequency(20*1000); // 20Khz
    float xmin,xmax;
    while(1)
    {
       // splash();
        tft->fillScreen(BLACK);   
        drawGrid();
        tft->setCursor(200, 160);
        tft->print(cal);
        
      //  for(int i=0;i<16;i++)
        {
            int count;
            int i=0;
            adc->initiateSampling(256);
            uint32_t *xsamples=adc->getSamples(count);
            for(int j=0;j<count;j++)
            {
                samples[j]=(int)(xsamples[j] >>16); //-cal; //&0xffff;
            }
            adc->reclaimSamples(xsamples);
            transform(samples,count,2289,voltageScale,xmin,xmax);
                      
             tft->setCursor(20, 120);
            tft->print(xmin);
            tft->setCursor(20, 140);
            tft->print(xmax);
            
            float last=samples[0]; // 00--4096
            last/=1024*256;
            if(last>239) last=239;
            if(last<0) last=0;
            
            for(int j=1;j<count;j++)
            {
                float next=samples[j]; // 1=1024 V
                next*=32;
                next/=1024; // 0..200
                if(next>239) next=239;
                if(next<0) next=0;
                
                if(next==last) last++;
                if(next>last)
                    tft->drawFastVLine(j,last,1+next-last,YELLOW);
                else
                    tft->drawFastVLine(j,next,1+last-next,YELLOW);
                last=next;
            }
        }
        tft->setCursor(200, 200);
        tft->print((int)avg);
        xDelay(300);
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
    v*=3.3;
    v/=4096; 
    voltageScale=v; // output in V
    
}
