
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
  commonTrigger(count,triggerValueADC);
  
  setupAdcDmaTransfer( requestedSamples,adcInternalBuffer, DMA1_CH1_TriggerEvent,true );  
  
  enableDisableIrqSource(true,ADC_AWD);    
  enableDisableIrq(true);
  startDMATime();
  volatile uint32_t s =ADC1->regs->DR;
  ADC_TIMER.resume();  
  return true;
} 
// EOF

