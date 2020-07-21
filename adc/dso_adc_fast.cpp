/**
 * Derived from https://github.com/pingumacpenguin/STM32-O-Scope/blob/master/STM32-O-Scope.ino
 */
/*.
(c) Andrew Hull - 2015
STM32-O-Scope - aka "The Pig Scope" or pigScope released under the GNU GENERAL PUBLIC LICENSE Version 2, June 1991
https://github.com/pingumacpenguin/STM32-O-Scope
Adafruit Libraries released under their specific licenses Copyright (c) 2013 Adafruit Industries.  All rights reserved.
*/
/**
 We use PA0 as input pin
 * DMA1, channel 0
 
 * Vref is using PWM mode for Timer4/Channel 3
 * 
 * Correct init order is 
 *     ADC
 *     DMA
 *     SWSTART
 * 
 */
#include "dso_adc.h"
#include "fancyLock.h"
#include "dma.h"
#include "adc.h"
#include "dso_adc_priv.h"
/**
 */

adc_reg_map *adc_Register;
volatile uint32_t cr2;


uint16_t DSOADC::adcInternalBuffer[ADC_INTERNAL_BUFFER_SIZE+16] __attribute__ ((aligned (8)));;;

int dmaSpuriousInterrupt=0;
extern HardwareTimer Timer4;


/**
 */
int requestedSamples;
uint32_t vcc; // power Supply in mv


FancySemaphore      *dmaSemaphore;
DSOADC             *instance=NULL;
/**
 * 
 */
DSOADC::DSOADC(int pin)
{
  instance=this;
  _pin=pin;

 
  // Set up our sensor pin(s)  
  dmaSemaphore=new FancySemaphore;  
  adc_Register=  PIN_MAP[_pin].adc_device->regs;
  
  
  enableDisableIrq(false);
  enableDisableIrqSource(false,ADC_AWD);
  enableDisableIrqSource(false,ADC_EOC);  
  
  setTriggerMode(DSOADC::Trigger_Run);
  attachWatchdogInterrupt(NULL);
 
  _oldTimerFq=0;
}
  
/**
 * 
 * @param adc
 * @return 
 */
float DSOADC::adcToVolt(float adc)
{
    adc=adc*vcc;
    adc/=4095000.;
    return adc;
}
void DSOADC::clearSamples()
{
    memset(adcInternalBuffer,0,sizeof(adcInternalBuffer));
}
/**
 */
bool    DSOADC::setADCPin(int pin)
{
    _pin=pin;
    adc_Register=  PIN_MAP[_pin].adc_device->regs;
    setChannel(PIN_MAP[_pin].adc_channel);
    return true;
}

// Grab the samples from the ADC
// Theoretically the ADC can not go any faster than this.
//
// According to specs, when using 72Mhz on the MCU main clock,the fastest ADC capture time is 1.17 uS. As we use 2 ADCs we get double the captures, so .58 uS, which is the times we get with ADC_SMPR_1_5.
// I think we have reached the speed limit of the chip, now all we can do is improve accuracy.
// See; http://stm32duino.com/viewtopic.php?f=19&t=107&p=1202#p1194

bool DSOADC::startDMA()
{    
  
  
  cr2=ADC1->regs->CR2;  
  cr2&= ~ADC_CR2_SWSTART;   
  ADC1->regs->CR2=cr2;
  cr2|=ADC_CR2_CONT+ADC_CR2_DMA;    
  ADC1->regs->CR2=cr2;    
  cr2|= ADC_CR2_SWSTART;   
  ADC1->regs->CR2=cr2;    
  return true;  
}
/**
 * 
 * @return 
 */
bool DSOADC::startDualDMA()
{    
 volatile uint32_t adc2;   
 // ADC -- 1 --
  cr2=ADC1->regs->CR2;
  cr2&= ~ADC_CR2_SWSTART;   
  ADC1->regs->CR2=cr2;
  cr2|=ADC_CR2_CONT+ADC_CR2_DMA;
  ADC1->regs->CR2=cr2;
  
  // ADC -- 2 --
  adc2=ADC2->regs->CR2;
  adc2 |=ADC_CR2_CONT;
  adc2 &=~ADC_CR2_DMA;
  ADC2->regs->CR2=adc2;
  
  // ADC -- 1 --
  cr2=ADC1->regs->CR2;
  cr2|= ADC_CR2_SWSTART;   
  ADC1->regs->CR2=cr2;
  return true;
  
}
#define XMAX(a,b) (((a)>(b))?(a):(b))
#define XMIN(a,b) (((a)<(b))?(a):(b))
/**
 * 
 * @param count
 * @return 
 */
bool DSOADC::startDMASampling (const int count)
{
  requestedSamples=XMIN(count,ADC_INTERNAL_BUFFER_SIZE);    
  enableDisableIrqSource(false,ADC_AWD);
  enableDisableIrq(true);  
  setupAdcDmaTransfer( requestedSamples,adcInternalBuffer, DMA1_CH1_Event,false );
  allAdcsOnOff(true);
  startDMA();
  return true;
}
/**
 * 
 * @param count
 * @return 
 */
bool DSOADC::startDualDMASampling (const int otherPin, const int count)
{
  requestedSamples=XMIN(count,ADC_INTERNAL_BUFFER_SIZE);    
  enableDisableIrqSource(false,ADC_AWD);
  enableDisableIrq(true);
  setupAdcDualDmaTransfer( otherPin, requestedSamples,(uint32_t *)adcInternalBuffer, DMA1_CH1_Event,false );
  startDualDMA();
  return true;
}


/**
 * 
 */
void SPURIOUS_INTERRUPT()
{
    
}


void DSOADC::stopDmaCapture(void)
{
    // disable interrupts
    cr2=  ADC1->regs->CR2 ;
    cr2 &= ~(ADC_CR2_SWSTART|ADC_CR2_CONT|ADC_CR2_DMA);   
    ADC1->regs->CR2 =cr2;
    enableDisableIrq(false);
    enableDisableIrqSource(false,ADC_AWD);
    // Stop dma
    adc_dma_disable(ADC1);
    allAdcsOnOff(false);
}


volatile uint32_t lastCR1=0;

#define SetCR1(x) {lastCR1=ADC1->regs->CR1=(x);}

voidFuncPtr adcIrqHandler=NULL;
/**
 */
uint32_t DSOADC::getVCCmv()
{
    return vcc;
}


/**
 * 
 * @return 
 */
bool    DSOADC::setupDmaSampling()
{   
  setOverSamplingFactor(1);    
  ADC_TIMER.pause();
  setSource(ADC_SOURCE_SWSTART);  
  return true;
}