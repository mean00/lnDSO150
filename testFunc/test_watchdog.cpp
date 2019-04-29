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
/**
 * 
 */
volatile bool done=false;
void dmaInterrupt()
{
    done=true;
}
void watchDog()
{
    
    sr=adc_Register->SR;
    if(!(sr&3)) return;
    uint32_t val=adc_Register->DR>>16;
    delta=micros()-mikro;
    interrupt=adc_Register->CR1;
    foobar=1;
    
    
}


void testAdcWatchdog(void)
{
    buffer=(uint32_t *)test_samples;
    adc_Register=  PIN_MAP[PA0].adc_device->regs;
    DSOCapture::setTimeBase(    DSOCapture::DSO_TIME_BASE_1MS);
    DSOCapture::setVoltageRange(DSOCapture::DSO_VOLTAGE_1V);
    while(1)
    {
          int ex=240;
          
          noInterrupts();
            dma_init(DMA1);
            dma_attach_interrupt(DMA1, DMA_CH1, dmaInterrupt);
            dma_setup_transfer(DMA1, DMA_CH1, &ADC1->regs->DR, DMA_SIZE_32BITS, buffer, DMA_SIZE_32BITS, (DMA_MINC_MODE | DMA_TRNS_CMPLT));// Receive buffer DMA
            dma_set_num_transfers(DMA1, DMA_CH1, 240 );            
            adc_Register->LTR=2500;
            adc_Register->HTR=4096;
            adc_Register->CR1 |= ADC_CR1_AWDEN;
            uint32_t val=adc_Register->DR; // clear
                     val=adc_Register->SR; // clear
            adc_enable_irq(ADC1,ADC_AWD);
            adc_attach_interrupt(ADC1, ADC_AWD, watchDog);
            
            nvic_irq_enable(NVIC_ADC_1_2 );
            done=false;
            mikro=micros();
            DSOADC::adc_dma_enable(ADC1);
            dma_enable(DMA1, DMA_CH1); // Enable the channel and start the transfer.
            interrupts();
            while(done==false)
            {
                xDelay(5);
            }
            DSOADC::adc_dma_disable(ADC1);
            
    }
  
}
