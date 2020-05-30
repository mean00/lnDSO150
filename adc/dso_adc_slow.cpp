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
extern adc_reg_map *adc_Register;

CaptureState captureState=Capture_idle;
/**
 */
extern int                  requestedSamples;
extern DSOADC               *instance;
       int                  currentIndex=0;
int spuriousTimer=0;

int nbSlowCapture=0;
static int skippedDma=0;
int nbTimer=0;
static int nbDma=0;



 uint16_t dmaOverSampleBuffer[DMA_OVERSAMPLING_COUNT] __attribute__ ((aligned (8)));;
extern void Oopps();


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
#if 1  
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
  setupAdcDmaTransfer( requestedSamples,adcInternalBuffer, DMA1_CH1_Event );
  startDMATime();
  ADC_TIMER.resume();  
  lastStartedCR2=ADC1->regs->CR2;
  return true;
}
/**
 * 
 * @return 
 */
bool    DSOADC::setupTimerSampling()
{   
  ADC_TIMER.pause();
  ADC_TIMER.setChannel1Mode(TIMER_OUTPUT_COMPARE); //TIMER_OUTPUT_COMPARE);
  ADC_TIMER.setMasterModeTrGo(TIMER_CR2_MMS_UPDATE); //TIMER_CR2_MMS_UPDATE); 
  setSource(ADC_SOURCE_TIMER);    
  setTimeScale(ADC_SMPR_7_5,DSOADC::ADC_PRESCALER_2); // slow enough sampling FQ, no need to be faster
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
    int  samplePeriodus =     1000000*10 / fq;
    ADC_TIMER.setPeriod(samplePeriodus/10);  
    int ov=ADC_TIMER.getOverflow();
    ADC_TIMER.setCompare1(ov-1);
    ADC_TIMER.refresh();
    _oldTimerFq=fq;
  }
  return true;    
}
/**
 * 
 * @param count
 * @param buffer
 * @return 
 */
bool DSOADC::startTimerSampling (int count)
{

   if(count>ADC_INTERNAL_BUFFER_SIZE/2)
        count=ADC_INTERNAL_BUFFER_SIZE/2;
    requestedSamples=count;

    currentIndex=0;    
    FancyInterrupts::disable();    
    captureState=Capture_armed;   
    startInternalDmaSampling();           
    FancyInterrupts::enable();
    return true;
} 
#include "dso_adc_slow_trigger.cpp"


// EOF

