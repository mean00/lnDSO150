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

/**
 * 
 * @param overSamp
 * @return 
 */ 
bool    DSOADC::setOverSamplingFactor  (int overSamp)
{   
    return true;
}

// EOF

