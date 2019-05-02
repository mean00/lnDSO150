/**
 * Derived from https://github.com/pingumacpenguin/STM32-O-Scope/blob/master/STM32-O-Scope.ino
 */

#include "dso_adc.h"

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
 */

static uint32_t cr1;
// Grab the samples from the ADC
// Theoretically the ADC can not go any faster than this.
//
// According to specs, when using 72Mhz on the MCU main clock,the fastest ADC capture time is 1.17 uS. As we use 2 ADCs we get double the captures, so .58 uS, which is the times we get with ADC_SMPR_1_5.
// I think we have reached the speed limit of the chip, now all we can do is improve accuracy.
// See; http://stm32duino.com/viewtopic.php?f=19&t=107&p=1202#p1194

void DSOADC::stopDmaCapture(void)
{
     adc_dma_disable(ADC1);
}

bool DSOADC::startDMATriggeredSampling (int count,int triggerValueADC)
{
  // This loop uses dual interleaved mode to get the best performance out of the ADCs
  //
    
  _triggered=false;
  if(count>ADC_INTERNAL_BUFFER_SIZE/2)
        count=ADC_INTERNAL_BUFFER_SIZE/2;
    
  requestedSamples=count;  
  convTime=micros();
  setWatchdogTriggerValue(1*2560+0*triggerValueADC,0);
  attachWatchdogInterrupt(DSOADC::watchDogInterrupt);  
  
  enableDisableIrqSource(true,ADC_AWD);
  enableDisableIrq(true);
  
  setupAdcDmaTransfer( requestedSamples,adcInternalBuffer, DMA1_CH1_TriggerEvent );
  
  cr1=ADC1->regs->CR1 ;
  
  return true;
}
/**
 * 
 */
void DSOADC::awdTrigger()
{
        _triggered=true;
        
}


/**
 * 
 */
void DSOADC::watchDogInterrupt()
{
    instance->awdTrigger();
    enableDisableIrq(false); // no more IRQ please
}
/**
 * 
 */
void DSOADC::DMA1_CH1_TriggerEvent() 
{
    if(instance->awdTriggered())
    {
        SampleSet one(requestedSamples,adcInternalBuffer),two(0,NULL);
        instance->captureComplete(true,one,two);
        adc_dma_disable(ADC1);
    }
    else
    {
        //nextAdcDmaTransfer(requestedSamples,adcInternalBuffer);
        setupAdcDmaTransfer( requestedSamples,adcInternalBuffer, DMA1_CH1_TriggerEvent );
    }
    
}
//
