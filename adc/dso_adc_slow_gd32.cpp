/**
 
 *  This is the slow capture mode
 * i.e. we setup n Samples acquisition through DMA
 * and a timer interrupt grabs the result 
 */

//#include "dso_global.h"
#include "dso_adc.h"
#include "dso_adc_priv.h"
#include "fancyLock.h"
#include "myPwm.h"

#define GD_OVERSAMPLING_ENABLED 1*1
#define GD_OVERSAMPLING_OVSR(x) (x<<2)
#define GD_OVERSAMPLING_OVSS(x) (x<<5)

#define GD_OVERSAMPLING_TOVS (1<<9)

int _overSamp=0;

/**
 *  This is called only once when we change mode
 * @return 
 */
bool    DSOADC::setupTimerSampling()
{   
  ADC_TIMER.pause();
  setSource(ADC_SOURCE_TIMER);    
// 239_5 =>This gives < 30 us sampling period, we are above 40 us  at 1ms/div
  _oldTimerFq=0;
  return true;
}
#warning OVERSAMPLING NOT SUPPORTED HERE
bool    DSOADC::prepareTimerSampling (int timerScale, int timerOvf,bool overSampling,adc_smp_rate adcRate , DSOADC::Prescaler adcScale)
{   
    int fq;
    ADC_TIMER.pause();
     pwmGetFrequency(  timerScale, timerOvf,fq);
     if(fq!=_oldTimerFq)
     {
        
       _oldTimerFq=fq;
       _timerSamplingRate=adcRate;
       _timerScale=adcScale;
       _overSampling=false;
       programTimer(  timerOvf,   timerScale);
     }  
    return true;    
}
/**
 * 
 * @param timerScale
 * @param timerOvf
 * @param overSampling
 * @param adcRate
 * @param adcScale
 * @return 
 */
bool    DSOADC::prepareDualTimerSampling (int timerScale, int timerOvf,bool overSampling,adc_smp_rate adcRate , DSOADC::Prescaler adcScale)
{       
   return   prepareTimerSampling(timerScale,timerOvf,overSampling,adcRate,adcScale);
}

/**
 * 
 * @param fq
 * @return 
 */
bool    DSOADC::prepareTimerSampling (int fq,bool overSampling,adc_smp_rate rate , DSOADC::Prescaler scale)
{   
  ADC_TIMER.pause();
  if(fq!=_oldTimerFq)
  {
    _oldTimerFq=fq;
    _timerSamplingRate=rate;
    _timerScale=scale;
    _overSampling=overSampling;
    // do a 4 time oversampling
    if(overSampling)
    {
      fq*=OVERSAMPLING_FACTOR;
      setOverSamplingFactor(OVERSAMPLING_FACTOR);        
    }else          
    {
        setOverSamplingFactor(1);
    }
    int scaler=F_CPU/(fq*65535);
    scaler+=1;
    int high=F_CPU/scaler;
    int overFlow=(high+fq/2)/fq;

    programTimer(  overFlow,   scaler);
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
    _overSamp=overSamp;
    // set adc1 off
    volatile adc_reg_map *regs=  ADC1->regs;       
    regs->CR2&=~1; 
    uint32_t o=*ovr;
    switch(overSamp)
    {
        case 1:     o=0;break; // oversampling off
        case 2 :    o=GD_OVERSAMPLING_ENABLED+GD_OVERSAMPLING_OVSR(0)+GD_OVERSAMPLING_OVSS(1)+GD_OVERSAMPLING_TOVS; break; // shift 1 bits, x2
        case 4 :    o=GD_OVERSAMPLING_ENABLED+GD_OVERSAMPLING_OVSR(1)+GD_OVERSAMPLING_OVSS(2)+GD_OVERSAMPLING_TOVS; break; // shift 2 bits, x4
        case 8 :    o=GD_OVERSAMPLING_ENABLED+GD_OVERSAMPLING_OVSR(2)+GD_OVERSAMPLING_OVSS(3)+GD_OVERSAMPLING_TOVS; break; // Shift 3 bits, x8
        default:
            xAssert(0);
            break;
    }
    *ovr=o;
    // adc on
    regs->CR2 |=1;
    return true;
}


/**
 * 
 * @return 
 */
int DSOADC::pollingRead()
{    
  adc_reg_map *regs=ADC1->regs;  
  uint32_t oldCr2=regs->CR2;
  
  // disable oversampling
  uint32_t oldOvr=_overSamp;
  setOverSamplingFactor(1);
  
  cr2=regs->CR2;
  cr2&= ~(ADC_CR2_SWSTART+ADC_CR2_CONT+ADC_CR2_DMA);   
  // Set source to SWSTART
   cr2=ADC1->regs->CR2;  
   cr2 &=~ ADC_CR2_EXTSEL_SWSTART;
   ADC1->regs->CR2=cr2;
   cr2 |= ((int)ADC_CR2_EXTSEL_SWSTART) << 17;
   ADC1->regs->CR2=cr2;         
   cr2=ADC1->regs->CR2;
  
  regs->CR2=cr2;
  // then poll
  cr2|=ADC_CR2_SWSTART;
  regs->CR2=cr2;
  // wait for end of sampling  
  while (!(regs->SR & ADC_SR_EOC))
  {
      
  }      
  uint16_t val= (uint16)(regs->DR & ADC_DR_DATA);
  regs->CR2=oldCr2;
  setOverSamplingFactor(oldOvr);
  return val;
}

// EOF

