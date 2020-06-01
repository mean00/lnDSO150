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

#define GD_OVERSAMPLING_ENABLED 1
#define GD_OVERSAMPLING_OVSR(x) (x<<2)
#define GD_OVERSAMPLING_OVSS(x) (x<<5)
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

/**
 * 
 * @param overSamp
 * @return 
 */ 
bool    DSOADC::setOverSamplingFactor  (int overSamp)
{
     __IO uint32 *ovr=(uint32_t *)(0x40012480);
    
    // set adc1 off
    volatile adc_reg_map *regs=  ADC1->regs;       
    regs->CR2&=~1; 
    
    switch(overSamp)
    {
        case 1:     *ovr=0;break; // oversampling off
        case 4 :    *ovr=GD_OVERSAMPLING_ENABLED+GD_OVERSAMPLING_OVSR(1)+GD_OVERSAMPLING_OVSS(2); break; // shift 2 bits, x4
        case 8 :    *ovr=GD_OVERSAMPLING_ENABLED+GD_OVERSAMPLING_OVSR(2)+GD_OVERSAMPLING_OVSS(3); break; // Shift 3 bits, x8
        default:
            xAssert(0);
            break;
    }
    // adc on
    regs->CR2 |=1;
    return true;
}

// EOF

