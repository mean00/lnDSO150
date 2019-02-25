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


/**
 * 
 */
void testCapture(void)
{
    DSODisplay::init();
    DSODisplay::drawGrid();
    int reCounter=0;
    
    tft->setTextSize(2);
    myTestSignal->setFrequency(10000); // 20Khz

    DSOCapture::setTimeBase(    DSOCapture::DSO_TIME_BASE_1MS);
    DSOCapture::setVoltageRange(DSOCapture::DSO_VOLTAGE_1V);
    float xmin,xmax,avg;
    
    while(1)
    {        
        int count=DSOCapture::oneShotCapture(240,samples);  
        DSOCapture::captureToDisplay(count,samples,waveForm);        
        DSODisplay::drawWaveForm(count,waveForm);
    }
} 


// EOF    
