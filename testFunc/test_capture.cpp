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

#include "dso_global.h"
#include "dsoDisplay.h"

extern void splash(void);
static void drawGrid(void);

//--
extern Adafruit_TFTLCD_8bit_STM32 *tft;
extern DSOControl *controlButtons;
extern testSignal *myTestSignal;
//

static uint8_t waveForm[240];
static float  samples[256];
static bool voltageMode=false;



static void redraw()
{
      DSODisplay::drawGrid();
      
        tft->setCursor(241, 100);
        tft->print(capture->getTimeBaseAsText());

        tft->setCursor(241, 140);
        tft->print(capture->getVoltageRangeAsText());
        
        tft->setCursor(241, 180);
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
            voltageMode=false;
            dirty=true;
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
                capture->setTimeBase( t);
            }
            // Redraw background
            dirty=true;
        }
        if(dirty)
            redraw();
}
/**
 * 
 */
void testCapture(void)
{
    DSODisplay::init();
   
    int reCounter=0;
    
    tft->setTextSize(2);
    myTestSignal->setFrequency(10000); // 20Khz

    DSOCapture::setTimeBase(    DSOCapture::DSO_TIME_BASE_1MS);
    DSOCapture::setVoltageRange(DSOCapture::DSO_VOLTAGE_1V);
    redraw();
    float xmin,xmax,avg;
    
    while(1)
    {        
        int count=DSOCapture::oneShotCapture(240,samples);  
        DSOCapture::captureToDisplay(count,samples,waveForm);        
        DSODisplay::drawWaveForm(count,waveForm);
        
        buttonManagement();        
    }
        
} 


// EOF    
