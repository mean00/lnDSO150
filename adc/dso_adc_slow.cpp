/**
 
 *  This is the slow capture mode
 * i.e. we setup n Samples acquisition through DMA
 * and a timer interrupt grabs the result 
 */

//#include "dso_global.h"
#include "dso_adc.h"
#include "dso_adc_priv.h"
#include "fancyLock.h"
#include "helpers/helper_pwm.h"
/**
 */
uint32_t lastStartedCR2=0;
uint32_t lastStartedCR1=0;
uint32_t lastStartedSR;
//

CaptureState captureState=Capture_idle;

/**
 * 
 * @param fqInHz
 * @return 
 */
bool DSOADC::setSlowMode(int fqInHz)
{    
    return true;
}

/**
 * 
 */
void DSOADC::stopTimeCapture(void)
{
     ADC_TIMER.pause();
     adc_dma_disable(ADC1);
}
/**
 * 
 * @return 
 */
bool DSOADC::startDMATime()
{    
#define USE_CONT 0
  cr2=ADC1->regs->CR2;  
  cr2&= ~(ADC_CR2_SWSTART+ADC_CR2_CONT);   
  ADC1->regs->CR2=cr2;
  setSourceInternal();   
  cr2|=ADC_CR2_CONT*USE_CONT+ADC_CR2_DMA;    
  ADC1->regs->CR2=cr2;    
#if 0  
  cr2|= ADC_CR2_SWSTART;   
  ADC1->regs->CR2=cr2;    
#endif  
  return true;  
}
/**
 * 
 * @param count
 * @param buffer
 * @return 
 */
bool DSOADC::startInternalDmaSampling ()
{
  //  slow is always single channel
  ADC1->regs->CR1&=~ADC_CR1_DUALMASK;
  setupAdcDmaTransfer( requestedSamples,adcInternalBuffer, DMA1_CH1_Event,false );
  
  startDMATime();
  volatile uint32_t s =ADC1->regs->DR;
  ADC_TIMER.resume();  
  lastStartedCR2=ADC1->regs->CR2;
  return true;
}
#ifdef HIGH_SPEED_ADC // GD32
/**
 *  This is called only once when we change mode
 * @return 
 */
bool    DSOADC::setupTimerSampling()
{   
  ADC_TIMER.pause();
  setSource(ADC_SOURCE_TIMER);    
// 239_5 =>This gives < 30 us sampling period, we are above 40 us  at 1ms/div
  setTimeScale(ADC_SMPR_71_5,DSOADC::ADC_PRESCALER_8); 
  setOverSamplingFactor(OVERSAMPLING_FACTOR);
  _oldTimerFq=0;
  return true;
}

/**
 * 
 * @param fq
 * @return 
 */
bool    DSOADC::prepareTimerSampling (int fq)
{   
  ADC_TIMER.pause();
  if(fq!=_oldTimerFq)
  {
      _oldTimerFq=fq;
      // do a 4 time oversampling
      fq*=OVERSAMPLING_FACTOR;
      int scaler=F_CPU/(fq*65535);
      scaler+=1;
      int high=F_CPU/scaler;
      int overFlow=(high+fq/2)/fq;

        ADC_TIMER.pause();
        ADC_TIMER.setPrescaleFactor(scaler);
        ADC_TIMER.setOverflow(overFlow);
        ADC_TIMER.setCompare(ADC_TIMER_CHANNEL,overFlow-1);
        timer_cc_enable(ADC_TIMER.c_dev(), ADC_TIMER_CHANNEL);
        
  }
  return true;    
}
#else

/**
 *  This is called only once when we change mode
 * @return 
 */
bool    DSOADC::setupTimerSampling()
{   
  ADC_TIMER.pause();
  setSource(ADC_SOURCE_TIMER);    
// 239_5 =>This gives < 30 us sampling period, we are above 40 us  at 1ms/div
  setTimeScale(ADC_SMPR_71_5,DSOADC::ADC_PRESCALER_8); 
  _oldTimerFq=0;
  return true;
}

/**
 * 
 * @param fq
 * @return 
 */
bool    DSOADC::prepareTimerSampling (int fq)
{   
  ADC_TIMER.pause();
  if(fq!=_oldTimerFq)
  {
      int scaler=F_CPU/(fq*65535);
      scaler+=1;
      int high=F_CPU/scaler;
      int overFlow=(high+fq/2)/fq;

        ADC_TIMER.pause();
        ADC_TIMER.setPrescaleFactor(scaler);
        ADC_TIMER.setOverflow(overFlow);
        ADC_TIMER.setCompare(ADC_TIMER_CHANNEL,overFlow-1);
        timer_cc_enable(ADC_TIMER.c_dev(), ADC_TIMER_CHANNEL);
        _oldTimerFq=fq;
  }
  return true;    
}
#endif
/**
 * 
 * @param count
 * @param buffer
 * @return 
 */
bool DSOADC::startTimerSampling (int count)
{

   if(count>ADC_INTERNAL_BUFFER_SIZE)
        count=ADC_INTERNAL_BUFFER_SIZE;
    requestedSamples=count;

    FancyInterrupts::disable();    
    captureState=Capture_armed;   
    startInternalDmaSampling();           
    FancyInterrupts::enable();
    return true;
} 

// EOF

