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


extern void splash(void);
static void drawGrid(void);

//--
extern Adafruit_TFTLCD_8bit_STM32 *tft;
extern DSOControl *controlButtons;
extern testSignal *myTestSignal;
//

static uint8_t waveForm[256]; // take a bit more, we have rounding issues
static bool voltageMode=false;


static void redraw()
{
      DSODisplay::drawGrid();
      
        tft->setCursor(241, 180);
        tft->print(capture->getTimeBaseAsText());

        tft->setCursor(241, 200);
        tft->print(capture->getVoltageRangeAsText());
        
        tft->setCursor(241, 220);
        if(voltageMode)
            tft->print("VOLT");
        else
            tft->print("TIME");
      
}


static void buttonManagement()
{
    bool dirty=false;
        int inc=controlButtons->getRotaryValue();
        if(controlButtons->getButtonEvents(DSOControl::DSO_BUTTON_VOLTAGE) & EVENT_SHORT_PRESS)
        {
            dirty=true;
            voltageMode=true;
        }
        if(controlButtons->getButtonEvents(DSOControl::DSO_BUTTON_TIME) & EVENT_SHORT_PRESS)
        {
            dirty=true;
            voltageMode=false;            
        }
        
        if(inc)
        {
            if(voltageMode)
            {
                int v=capture->getVoltageRange();
                v+=inc;
                if(v<0) v=0;
                if(v>DSOCapture::DSO_VOLTAGE_MAX) v=DSOCapture::DSO_VOLTAGE_MAX;
                capture->setVoltageRange((DSOCapture::DSO_VOLTAGE_RANGE)v);                                
            }
            else // Timingmode
            {
                 int v=capture->getTimeBase();
                 v+=inc;
                if(v<0) v=0;
                if(v>DSOCapture::DSO_TIME_BASE_MAX) v=DSOCapture::DSO_TIME_BASE_MAX;
                DSOCapture::DSO_TIME_BASE  t=(DSOCapture::DSO_TIME_BASE )v;
                DSOCapture::clearCapturedData();
                capture->setTimeBase( t);
                
            }
            // Redraw background
            dirty=true;
        }
        if(dirty)
            redraw();
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
static float voltageSamples[240];
/**
 * 
 */
void testCapture(void)
{
    DSODisplay::init();
    CaptureStats stats;
    int reCounter=0;
    
    tft->setTextSize(2);
    myTestSignal->setFrequency(100); // 100-> 10ms

    DSOCapture::setTimeBase(    DSOCapture::DSO_TIME_BASE_10MS);
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
    
    while(1)
    {        
        int lastTime=millis();
#if 1        
        int count=DSOCapture::triggeredCapture(240,voltageSamples,stats);  
#else
          int count=DSOCapture::oneShotCapture(240,voltageSamples,stats);  
#endif
        if(!count) 
        {
            buttonManagement();
            continue;
        }
        DSOCapture::captureToDisplay(count,voltageSamples,waveForm);  
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
        DSODisplay::drawVoltageTrigger(true,triggerLine);
        if(!counter)
            drawStats(stats);
        buttonManagement();        

        
        lastTime=millis()-lastTime;
        lastTime=100000/(lastTime+1);
        tft->setCursor(241, 160);
        //if(lastTime<4500)
            tft->print(lastTime);
    }
        
} 


// EOF    
