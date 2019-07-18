/**
 * Derived from https://github.com/pingumacpenguin/STM32-O-Scope/blob/master/STM32-O-Scope.ino
 */

#include "dso_adc.h"
#include "dso_capture_priv.h"

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
#define ADC_MAX 0xfff // 12 bits

static uint32_t cr1;
// Grab the samples from the ADC
// Theoretically the ADC can not go any faster than this.
//
// According to specs, when using 72Mhz on the MCU main clock,the fastest ADC capture time is 1.17 uS. As we use 2 ADCs we get double the captures, so .58 uS, which is the times we get with ADC_SMPR_1_5.
// I think we have reached the speed limit of the chip, now all we can do is improve accuracy.
// See; http://stm32duino.com/viewtopic.php?f=19&t=107&p=1202#p1194

void DSOADC::stopDmaCapture(void)
{
    // disable interrupts
    enableDisableIrq(false);
    enableDisableIrqSource(false,ADC_AWD);
    enableDisableIrqSource(false,ADC_EOC);
    // Stop dma
     adc_dma_disable(ADC1);
}

bool DSOADC::startDMATriggeredSampling (int count,int triggerValueADC)
{
  // This loop uses dual interleaved mode to get the best performance out of the ADCs
  //
  enableDisableIrq(false);
  adcInterruptStats.start();
  ADC1->regs->SR=0; // Clear all pending bits  
  _triggered=false;
  if(count>ADC_INTERNAL_BUFFER_SIZE/2)
        count=ADC_INTERNAL_BUFFER_SIZE/2;
    
  int currentValue=analogRead(analogInPin);
  
  requestedSamples=count;  
  convTime=micros();
  _triggerValueADC=triggerValueADC;
  switch(_triggerMode)
  {
    case Trigger_Rising:  
                        if(currentValue<triggerValueADC) 
                        {
                            _triggerState=Trigger_Armed;
                            setWatchdogTriggerValue(triggerValueADC,0);
                        }else
                        {
                            _triggerState=Trigger_Preparing;
                            setWatchdogTriggerValue(ADC_MAX,triggerValueADC);
                        }
                        break;
    case Trigger_Falling: 
                        if(currentValue>triggerValueADC) 
                        {
                            _triggerState=Trigger_Armed;
                            setWatchdogTriggerValue(ADC_MAX,triggerValueADC);
                        }else
                        {
                            _triggerState=Trigger_Preparing;
                            setWatchdogTriggerValue(triggerValueADC,0);
                        }
                        break;
    
    
    case Trigger_Both:    
                        _triggerState=Trigger_Armed;
                        if(currentValue>triggerValueADC) 
                        {
                            setWatchdogTriggerValue(ADC_MAX,triggerValueADC);
                        }else
                        {
                            setWatchdogTriggerValue(triggerValueADC,0);
                        }
                        break;
    case Trigger_Run:
                        break;
                        
    default: break;
  }  
   
  
  attachWatchdogInterrupt(DSOADC::watchDogInterrupt);  
  cr1=ADC1->regs->CR1 ;
  ADC1->regs->SR=0;
  enableDisableIrqSource(true,ADC_AWD);
  enableDisableIrqSource(false,ADC_EOC); // we dont need interrupt, it is done through DMA
  cr1=ADC1->regs->CR1 ;
  setupAdcDmaTransfer( ADC_INTERNAL_BUFFER_SIZE,adcInternalBuffer, DMA1_CH1_TriggerEvent );  
  enableDisableIrq(true);
  
  
  return true;
}
/**
 * 
 */
void DSOADC::awdTrigger()
{
     if(_triggerState==Trigger_Preparing)
     {
         _triggerState=Trigger_Armed;
         switch(_triggerMode)
         {
            case Trigger_Rising:
                setWatchdogTriggerValue(_triggerValueADC,0);
                break;
            case Trigger_Falling:
                setWatchdogTriggerValue(ADC_MAX,_triggerValueADC);
                 break;
            case Trigger_Both:
                // Tricky !
                setWatchdogTriggerValue(ADC_MAX,_triggerValueADC); // FIXME!
                 break;
            case Trigger_Run:
                _triggered=true;   
                 enableDisableIrqSource(false,ADC_AWD);
                 break;
         }
     }else
     {
        _triggered=true;   
        enableDisableIrqSource(false,ADC_AWD);
     }
}


/**
 * 
 */
void DSOADC::watchDogInterrupt()
{
    adcInterruptStats.watchDog++;
    instance->awdTrigger();
    
}
/**
 * 
 */
void DSOADC::DMA1_CH1_TriggerEvent() 
{
    adcInterruptStats.adcEOC++;
    xAssert(adcInterruptStats.adcEOC<2);
    if(instance->awdTriggered())
    {
        adcInterruptStats.eocTriggered++;
        enableDisableIrq(false);
        adc_dma_disable(ADC1);       
        SampleSet one(ADC_INTERNAL_BUFFER_SIZE,adcInternalBuffer),two(0,NULL);
        instance->captureComplete(one,two);
    }
    else
    {
        adcInterruptStats.eocIgnored++;
        //nextAdcDmaTransfer(requestedSamples,adcInternalBuffer);
        setupAdcDmaTransfer( requestedSamples,adcInternalBuffer, DMA1_CH1_TriggerEvent );
    }
    
}
//

  