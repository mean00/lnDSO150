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
#include "dso_adc.h"
#include "dso_capture.h"
#include "dso_global.h"
extern void splash(void);

static void drawGrid(void);
//--
extern Adafruit_TFTLCD_8bit_STM32 *tft;
extern DSOControl *controlButtons;
extern int ints;
extern DSOADC    *adc;
static adc_reg_map *adc_Register;
static uint32_t sr=0;
static uint32_t mikro,delta;
static int foobar;
static int interrupt;
extern float *test_samples;
uint32_t *buffer;
int nbDma=0;
int nbAwd=0;
/**
 * 
 */
volatile bool done=false;
void dmaInterrupt()
{
    done=true;
    nbDma++;
}
void watchDog()
{
    nbAwd++;
}


void testAdcWatchdog(void)
{
    buffer=(uint32_t *)test_samples;
    adc_Register=  PIN_MAP[analogInPin].adc_device->regs;
    DSOCapture::setTimeBase(    DSOCapture::DSO_TIME_BASE_1MS);
    DSOCapture::setVoltageRange(DSOCapture::DSO_VOLTAGE_1V);
    tft->setTextSize(2);
   
#if 1    
    DSOADC::enableDisableIrq(false);
    uint32_t val=adc_Register->DR; // clear
             val=adc_Register->SR; // clear

            
    DSOADC::setWatchdogTriggerValue(2000,0);            
    DSOADC::attachWatchdogInterrupt(watchDog);
    //DSOADC::enableDisableIrqSource(true,ADC_EOC);
    DSOADC::enableDisableIrqSource(true,ADC_AWD);
    DSOADC::enableDisableIrq(true);            

#endif
    done=false;
    mikro=micros();

    DSOADC::setupAdcDmaTransfer(   240,buffer, dmaInterrupt )   ;        
    
    while(1)
    {
        int ex=240;
        while(done==false)
        {
            xDelay(5);
        }
        done=false;
        DSOADC::adc_dma_disable(ADC1);
        DSOADC::setupAdcDmaTransfer( 240,buffer,dmaInterrupt)  ;
        
        tft->fillScreen(0);
        tft->setCursor(240, 100);
        tft->print(nbDma);
        tft->setCursor(240, 120);
        tft->print(nbAwd);
        
    }
  
}
