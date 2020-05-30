
#include "dso_adc.h"
#include "dso_adc_priv.h"
#include "fancyLock.h"

/**
 * 
 * @param count
 * @param buffer
 * @return 
 */
bool DSOADC::startTriggeredTimerSampling (int count,uint32_t triggerValueADC)
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
  
  setupAdcDmaTransfer( requestedSamples,adcInternalBuffer, DMA1_CH1_TriggerEvent );  
  
  enableDisableIrqSource(true,ADC_AWD);    
  enableDisableIrq(true);
  startDMATime();
  ADC_TIMER.resume();  
  return true;
} 
// EOF

