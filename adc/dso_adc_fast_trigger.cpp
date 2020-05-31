/**
 * Derived from https://github.com/pingumacpenguin/STM32-O-Scope/blob/master/STM32-O-Scope.ino
 */

#include "dso_adc.h"
#include "dso_capture_priv.h"
#include "dso_adc_priv.h"

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



// Grab the samples from the ADC
// Theoretically the ADC can not go any faster than this.
//
// According to specs, when using 72Mhz on the MCU main clock,the fastest ADC capture time is 1.17 uS. As we use 2 ADCs we get double the captures, so .58 uS, which is the times we get with ADC_SMPR_1_5.
// I think we have reached the speed limit of the chip, now all we can do is improve accuracy.
// See; http://stm32duino.com/viewtopic.php?f=19&t=107&p=1202#p1194

/**
 * 
 * @return 
 */
#define NB_REG 14
static volatile uint32_t reg[NB_REG];


void DSOADC::resetStats()
{
}

void DSOADC::getRegisters(void)
{
    __IO uint32_t  *p=(__IO uint32_t *)ADC1->regs;
    for(int i=0;i<NB_REG;i++)
    {
       reg[i]=p[i]; 
    }
}
/**
 */
bool DSOADC::commonTrigger (int count,uint32_t triggerValueADC)
{
    // This loop uses dual interleaved mode to get the best performance out of the ADCs
  //
  enableDisableIrq(false);
  
  _triggered=false;
    int  currentValue=0;
  
  currentValue=pollingRead();
  
  requestedSamples=count;  
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
                            _both=Trigger_Falling;
                            setWatchdogTriggerValue(ADC_MAX,triggerValueADC);
                        }else
                        {
                            _both=Trigger_Rising;
                            setWatchdogTriggerValue(triggerValueADC,0);
                        }
                        break;
    case Trigger_Run:
                        break;
                        
    default: break;
  }  
   
  
  attachWatchdogInterrupt(DSOADC::watchDogInterrupt);  
  
  ADC1->regs->SR=0; 
  return true;
}
/**
 * 
 * @param count
 * @param triggerValueADC
 * @return 
 */
bool DSOADC::startDMATriggeredSampling (int count,int triggerValueADC)
{
  commonTrigger(count,triggerValueADC);
  bool dual=false;
  if(_dual!=DSOADC::ADC_CAPTURE_MODE_NORMAL)
  {
        setupAdcDualDmaTransfer( _pin, requestedSamples,(uint32_t *)adcInternalBuffer, DMA1_CH1_TriggerEvent,true );
        dual=true;
  }
  else
  {
        setupAdcDmaTransfer( requestedSamples,adcInternalBuffer, DMA1_CH1_TriggerEvent,true );  
  }
  enableDisableIrqSource(true,ADC_AWD);    
  enableDisableIrq(true);
  if(dual)
      startDualDMA();
  else
      startDMA();
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
         switch(getActualTriggerMode())
         {
            case Trigger_Rising:
                setWatchdogTriggerValue(_triggerValueADC,0);
                break;
            case Trigger_Falling:
                setWatchdogTriggerValue(ADC_MAX,_triggerValueADC);
                 break;
            case Trigger_Both:
                xAssert(0); // should be _both and either Rising or falling
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
    
    instance->awdTrigger();
    
}
/**
 * 
 */
void DSOADC::DMA1_CH1_TriggerEvent() 
{

    if(instance->awdTriggered())
    {   
        enableDisableIrq(false);
        adc_dma_disable(ADC1);       
        SampleSet one(ADC_INTERNAL_BUFFER_SIZE,adcInternalBuffer),two(0,NULL);
        instance->captureComplete(one,two);
    }  
    
}
//

  
