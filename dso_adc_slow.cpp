/**
 
 *  This is the slow capture mode
 * i.e. we setup n Samples acquisition through DMA
 * and a timer interrupt grabs the result 
 */

#include "dso_global.h"

/**
 */

#define analogInPin  PA0


extern HardwareTimer Timer2;
extern adc_reg_map *adc_Register;

/**
 */
extern int requestedSamples;
extern uint32_t *currentSamplingBuffer;
extern DSOADC             *instance;
int currentIndex=0;
extern uint32_t convTime;
int spurious=0;
int notRunning=0;
bool timerRunning=false;
int nbSlowCapture=0;


extern SampleSet *currentSet;

#define DMA_OVERSAMPLING_COUNT 4
static uint32_t dmaOverSampleBuffer[DMA_OVERSAMPLING_COUNT] __attribute__ ((aligned (8)));;


/**
 * 
 * @param fqInHz
 * @return 
 */
bool DSOADC::setSlowMode(int fqInHz)
{    
    Timer2.attachInterrupt(TIMER_CH1, Timer2_Event);
    Timer2.setPeriod(1000000/fqInHz); // in microseconds
    Timer2.setCompare(TIMER_CH1, 1);    
}
/*
 * Can we skip that ?
 */
static void dummy_dma_interrupt_handler(void)
{
    
}

/**
 * 
 * @param count
 * @param buffer
 * @return 
 */
bool DSOADC::startInternalDmaSampling ()
{
  dma_attach_interrupt(DMA1, DMA_CH1, dummy_dma_interrupt_handler);  
  dma_init(DMA1);    
  dma_setup_transfer(DMA1, DMA_CH1, &ADC1->regs->DR, DMA_SIZE_32BITS, dmaOverSampleBuffer, DMA_SIZE_32BITS, (DMA_MINC_MODE | DMA_TRNS_CMPLT));// Receive buffer DMA
  dma_set_num_transfers(DMA1, DMA_CH1, DMA_OVERSAMPLING_COUNT );
  adc_dma_enable(ADC1);
  dma_enable(DMA1, DMA_CH1); // Enable the channel and start the transfer.
  return true;
}
  /**
  * 
  * @param count
  * @return 
  */
bool    DSOADC::prepareTimerSampling (int fq)
{        
    setSlowMode(fq);        
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
    if(!capturedBuffers.empty())
        return true; // We have data !
    
    currentSet=availableBuffers.take();
    if(!currentSet) return false;    

    if(count>maxSamples)
        count=maxSamples;   
    requestedSamples=count;
    currentSet->samples=count;
    currentSamplingBuffer=currentSet->data;
    currentIndex=0;
    convTime=micros();
    timerRunning=true;
    startInternalDmaSampling();        
    Timer2.setMode(TIMER_CH1, TIMER_OUTPUTCOMPARE); // start timer
    
} 
void DSOADC::Timer2_Event() 
{
    instance->timerCapture();
}
/**
 * \fn timerCapture
 * \brief this is one is called by a timer interrupt
 */
void DSOADC::timerCapture()
{    
    uint32_t avg=0;
    if(!timerRunning)
    {
        notRunning++;
        return;
    }
    if(!currentSamplingBuffer)
    {
        spurious++;
        return; // spurious interrupt
    }
    for(int i=0;i<DMA_OVERSAMPLING_COUNT;i++)
        avg+=dmaOverSampleBuffer[i];
    avg=(avg+DMA_OVERSAMPLING_COUNT/2+1)/DMA_OVERSAMPLING_COUNT;
    currentSamplingBuffer[currentIndex]=avg;
    currentIndex++;
    if(currentIndex>requestedSamples)
    {
        currentSet->samples=currentIndex;
        timerRunning=false;
        dma_disable(DMA1, DMA_CH1);
        Timer2.setMode(TIMER_CH1,TIMER_DISABLED);
        captureComplete();
        return;
    }
    // Ask for next set of samples
    dma_setup_transfer(DMA1, DMA_CH1, &ADC1->regs->DR, DMA_SIZE_32BITS, dmaOverSampleBuffer, DMA_SIZE_32BITS, (DMA_MINC_MODE | DMA_TRNS_CMPLT));// Receive buffer DMA
    dma_set_num_transfers(DMA1, DMA_CH1, DMA_OVERSAMPLING_COUNT );
    dma_enable(DMA1, DMA_CH1); // Enable the channel and start the transfer.
}

// EOF

