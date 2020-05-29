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
/*
 * Can we skip that ?
 */
static void dummy_dma_interrupt_handler(void)
{
    if(captureState!=Capture_armed)
        Oopps();
    captureState=Capture_dmaDone;
    nbDma++;
}
/**
 * 
 */
void DSOADC::stopTimeCapture(void)
{
     ADC_TIMER.pause();
     adc_dma_disable(ADC1);
     ADC_TIMER.attachInterrupt(ADC_TIMER_CHANNEL, NULL);
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
  startDMA();
  lastStartedCR2=ADC1->regs->CR2;
  return true;
}
  /**
  * 
  * @param count
  * @return 
  */
bool    DSOADC::prepareTimerSampling (int fq)
{         
    setTimerFrequency(&ADC_TIMER,ADC_TIMER_CHANNEL, fq);;  
    setTimeScale(ADC_SMPR_28_5,DSOADC::ADC_PRESCALER_6); // slow enough sampling FQ, no need to be faster
    setSource(ADC_SOURCE_TIMER);    
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

