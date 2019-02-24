/**
 
 *  This is the slow capture mode
 *  i.e. we use a timer to get samples
 * Only used when fq is >> compared to maximum sampling time 
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


#define DMA_OVERSAMPLING_COUNT 4
static uint32_t dmaOverSampleBuffer[DMA_OVERSAMPLING_COUNT];


/**
 * 
 * @param fqInHz
 * @return 
 */
bool DSOADC::setSlowMode(int fqInHz)
{
    
    Timer2.setChannel1Mode(TIMER_OUTPUTCOMPARE);
    Timer2.setPeriod(1000000/fqInHz); // in microseconds
    Timer2.attachCompare1Interrupt(Timer2_Event);
    Timer2.setCompare1(1); // overflow might be small
}
/**
 * 
 * @param count
 * @param buffer
 * @return 
 */
bool DSOADC::startInternalDmaSampling ()
{
  dma_detach_interrupt(DMA1,DMA_CH1);    
  dma_init(DMA1);  
  adc_dma_enable(ADC1);
  dma_setup_transfer(DMA1, DMA_CH1, &ADC1->regs->DR, DMA_SIZE_32BITS, dmaOverSampleBuffer, DMA_SIZE_32BITS, (DMA_MINC_MODE | DMA_TRNS_CMPLT));// Receive buffer DMA
  dma_set_num_transfers(DMA1, DMA_CH1, DMA_OVERSAMPLING_COUNT );
  dma_enable(DMA1, DMA_CH1); // Enable the channel and start the transfer.
  return true;
}
  /**
  * 
  * @param count
  * @return 
  */
bool    DSOADC::initiateTimerSampling (int count)
{    
    pinMode(analogInPin, INPUT_ANALOG);
    if(!capturedBuffers.empty())
        return true; // We have data !
    
    uint32_t *bfer=availableBuffers.take();
    if(!bfer) 
        return false;
    
    return startTimerSampling(count,bfer);
    
}
/**
 * 
 * @param count
 * @param buffer
 * @return 
 */
bool DSOADC::startTimerSampling (int count,uint32_t *buffer)
{
    if(count>maxSamples)
        count=maxSamples;   
    requestedSamples=count;
    currentSamplingBuffer=buffer;
    currentIndex=0;
    convTime=micros();
    startInternalDmaSampling();
    setSlowMode(4800);
    
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
    for(int i=0;i<DMA_OVERSAMPLING_COUNT;i++)
        avg+=dmaOverSampleBuffer[i];
    avg/=DMA_OVERSAMPLING_COUNT;
    currentSamplingBuffer[currentIndex]=avg;
    currentIndex++;
    if(currentIndex>requestedSamples)
    {
        dma_disable(DMA1, DMA_CH1);
        Timer2.detachInterrupt(1);        
        captureComplete();
        return;
    }
    dma_init(DMA1); 
    dma_setup_transfer(DMA1, DMA_CH1, &ADC1->regs->DR, DMA_SIZE_32BITS, dmaOverSampleBuffer, DMA_SIZE_32BITS, (DMA_MINC_MODE | DMA_TRNS_CMPLT));// Receive buffer DMA
    dma_set_num_transfers(DMA1, DMA_CH1, DMA_OVERSAMPLING_COUNT );
    dma_enable(DMA1, DMA_CH1); // Enable the channel and start the transfer.

}

// EOF

