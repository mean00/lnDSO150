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
#include "dso_control.h"
#include "HardwareSerial.h"

#include "dso_global.h"
#include "dso_display.h"
#include "dso_adc.h"

extern void splash(void);
static void drawGrid(void);

//--
extern Adafruit_TFTLCD_8bit_STM32 *tft;
extern DSOControl *controlButtons;
extern testSignal *myTestSignal;
extern DSOADC   *adc;
//
extern float test_samples[256];
static uint8_t waveForm[256]; // take a bit more, we have rounding issues



typedef enum              
{
            VOLTAGE_MODE,
            TIME_MODE,
            TRIGGER_MODE
}MODE_TYPE;

static MODE_TYPE mode=VOLTAGE_MODE;
 

static void redraw()
{
      DSODisplay::drawGrid();
      
        tft->setCursor(241, 180);
        tft->print(capture->getTimeBaseAsText());

        tft->setCursor(241, 200);
        tft->print(capture->getVoltageRangeAsText());
        
        tft->setCursor(241, 220);
        switch(mode)
        {
            case VOLTAGE_MODE: tft->print("VOLT");break;
            case TIME_MODE: tft->print("TIME");break;
            case TRIGGER_MODE: tft->print("TRIGG");break;
            default:  tft->print("xxxx");break;
        }
      
}
#define REFRESH() DSOCapture::stopCapture()

static void buttonManagement()
{
    bool dirty=false;
        int inc=controlButtons->getRotaryValue();
        if(controlButtons->getButtonEvents(DSOControl::DSO_BUTTON_VOLTAGE) & EVENT_SHORT_PRESS)
        {
            dirty=true;
            mode=VOLTAGE_MODE;
        }
        if(controlButtons->getButtonEvents(DSOControl::DSO_BUTTON_TIME) & EVENT_SHORT_PRESS)
        {
            dirty=true;
             mode=TIME_MODE;         
        }
        if(controlButtons->getButtonEvents(DSOControl::DSO_BUTTON_TRIGGER) & EVENT_SHORT_PRESS)
        {
            dirty=true;
            mode=TRIGGER_MODE;         
        }

        if(dirty)
        {
            redraw();
        }
        dirty=false;
        
        if(inc)
        {
            
            switch(mode)
            {
                case VOLTAGE_MODE: 
                                    {
                                    int v=capture->getVoltageRange();
                                        dirty=true;
                                    v+=inc;
                                    if(v<0) v=0;
                                    if(v>DSOCapture::DSO_VOLTAGE_MAX) v=DSOCapture::DSO_VOLTAGE_MAX;
                                    capture->setVoltageRange((DSOCapture::DSO_VOLTAGE_RANGE)v);                                
                                    REFRESH();
                                }
                                break;
                case TIME_MODE: 
                                {
                                    int v=capture->getTimeBase();
                                        dirty=true;
                                    v+=inc;
                                   if(v<0) v=0;
                                   if(v>DSOCapture::DSO_TIME_BASE_MAX) v=DSOCapture::DSO_TIME_BASE_MAX;
                                   DSOCapture::DSO_TIME_BASE  t=(DSOCapture::DSO_TIME_BASE )v;
                                   DSOCapture::clearCapturedData();
                                   capture->setTimeBase( t);
                                   REFRESH();
                                }
                                break;
                case TRIGGER_MODE: 
                                {
                                 float v=capture->getTriggerValue();
                                   
                                    v+=0.1*(float)inc;
                                   capture->setTriggerValue(v);    
                                   dirty=true;
                                   REFRESH();
                                    
                                    
                                }
                                break;
                default: 
                                break;
            }       
        if(dirty)
            redraw();
        }
}

static void drawStats(CaptureStats &stats)
{
#define AND_ONE(x,y) {    tft->setCursor(242, y*20); tft->print(x);}    
    
    AND_ONE("Min",0);
    AND_ONE(stats.xmin,1);
    AND_ONE("Max",2);
    AND_ONE(stats.xmax,3);
    AND_ONE("Avg",4);
    AND_ONE(stats.avg,5);
    
}
/**
 */
static const char *fq2Text(int fq)
{
    static char buff[16];
    float f=fq;
    const char *suff="";
#define STEP(x,t)  if(f>x)     {suff=t;f/=x;}else

    STEP(1000000,"M")
    STEP(1000,"K")
    {}
    
    sprintf(buff,"%3.1f%sHz",f,suff);
    return buff;
}
/**
 * 
 */
void mainDSOUI(void)
{
    DSODisplay::init();
    CaptureStats stats;
    int reCounter=0;
    
    tft->setTextSize(2);
    myTestSignal->setFrequency(1000); // 1 khz

    DSOCapture::setTimeBase(    DSOCapture::DSO_TIME_BASE_1MS);
    DSOCapture::setVoltageRange(DSOCapture::DSO_VOLTAGE_1V);
    redraw();
    float xmin,xmax,avg;
    
    int counter=0;
    int lastTrigger=-1;
    int triggerLine;
    
    DSOCapture::setTriggerValue(3.);
    float f=DSOCapture::getTriggerValue();
    triggerLine=DSOCapture::voltageToPixel(f);
    
    // 
    int old=millis();
    int avgFq=0;
    int nbFq=0;
    
    while(1)
    {        
        int lastTime=millis();
#if 1        
        int count=DSOCapture::triggeredCapture(240,test_samples,stats);  
#else
          int count=DSOCapture::oneShotCapture(240,test_samples,stats);  
#endif
        if(!count) 
        {
            DSODisplay::drawVoltageTrigger(false,triggerLine);
            buttonManagement();
            float f=DSOCapture::getTriggerValue();
            triggerLine=DSOCapture::voltageToPixel(f);
            DSODisplay::drawVoltageTrigger(true,triggerLine);
            continue;
        }
        DSOCapture::captureToDisplay(count,test_samples,waveForm);  
        // Remove trigger
        DSODisplay::drawVoltageTrigger(false,triggerLine);
        
        DSODisplay::drawWaveForm(count,waveForm);
        counter=(counter+1)%8;
        
        if(lastTrigger!=-1)
        {
             DSODisplay::drawVerticalTrigger(false,lastTrigger);
             lastTrigger=-1;
        }
        
        if(stats.trigger!=-1)
        {
            lastTrigger=stats.trigger;
            DSODisplay::drawVerticalTrigger(true,lastTrigger);
        }
        float f=DSOCapture::getTriggerValue();
        triggerLine=DSOCapture::voltageToPixel(f);
        DSODisplay::drawVoltageTrigger(true,triggerLine);
        if(!counter)
            drawStats(stats);
        buttonManagement();        
        
        tft->setCursor(241, 120);  
        if(stats.frequency>0)
        {
            avgFq+=stats.frequency;
            nbFq++;
            if(millis()-old>1000) // refresh every sec
            {        
                tft->print(fq2Text(stats.frequency));
                nbFq=0;
                avgFq=0;
                old=millis();
            }
        }
        lastTime=millis()-lastTime;
        lastTime=100000/(lastTime+1);
        tft->setCursor(241, 160);
        //if(lastTime<4500)
            tft->print(lastTime);
    }
        
} 


// EOF    

