/**
 
 *  This is the slow capture mode
 * i.e. we setup n Samples acquisition through DMA
 * and a timer interrupt grabs the result 
 */

#include "dso_global.h"

/**
 */

#define analogInPin  PA0

#define CAPTURE_TIMER_CHANNEL TIMER_CH1

extern HardwareTimer Timer2;
extern adc_reg_map *adc_Register;

enum CaptureState
{
    Capture_idle,
    Capture_armed,
    Capture_dmaDone,
    Capture_timerDone,
    Capture_complete
};
static CaptureState captureState=Capture_idle;
/**
 */
extern int                  requestedSamples;
extern uint32_t             *currentSamplingBuffer;
extern DSOADC               *instance;
       int                  currentIndex=0;
extern uint32_t             convTime;
int spuriousTimer=0;

int nbSlowCapture=0;
static int skippedDma=0;
static int nbTimer=0;
static int nbDma=0;


extern SampleSet *currentSet;

#define DMA_OVERSAMPLING_COUNT 4
static uint32_t dmaOverSampleBuffer[DMA_OVERSAMPLING_COUNT] __attribute__ ((aligned (8)));;
extern void Oopps();


/**
 * 
 * @param fqInHz
 * @return 
 */
bool DSOADC::setSlowMode(int fqInHz)
{    
    Timer2.attachInterrupt(CAPTURE_TIMER_CHANNEL, Timer2_Event);
    Timer2.setPeriod(1000000/(fqInHz*16)); // in microseconds, oversampled 16 times
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
    setTimeScale(ADC_SMPR_55_5,ADC_PRE_PCLK2_DIV_2);
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
    
    noInterrupts();
    captureState=Capture_armed;
    startInternalDmaSampling();   
    
    Timer2.setCompare(CAPTURE_TIMER_CHANNEL, 16);    
    Timer2.setMode(CAPTURE_TIMER_CHANNEL, TIMER_OUTPUTCOMPARE); // start timer
    Timer2.refresh();
    Timer2.resume();
    
    interrupts();
} 
void DSOADC::Timer2_Event() 
{    
    nbTimer++;
    instance->timerCapture();
}
/**
 * \fn timerCapture
 * \brief this is one is called by a timer interrupt
 */
void DSOADC::timerCapture()
{    
    uint32_t avg=0;
    switch(captureState)
    {
        case Capture_armed: // skipped one ADC DMA ?
            skippedDma++;
            return;
        case Capture_dmaDone:
            captureState=Capture_timerDone;
            break;
        case Capture_timerDone:
            spuriousTimer++;
            //Oopps();
            return;
            break;
        case Capture_complete:
            break;
        default:
            Oopps();
            break;
    }
    
    if(!currentSamplingBuffer)
    {
        Oopps();
        return; // spurious interrupt
    }
    for(int i=0;i<DMA_OVERSAMPLING_COUNT;i++)
        avg+=dmaOverSampleBuffer[i];
    avg=(avg+DMA_OVERSAMPLING_COUNT/2+1)/DMA_OVERSAMPLING_COUNT;
    currentSamplingBuffer[currentIndex]=avg;
    currentIndex++;
    if(currentIndex>=requestedSamples)
    {
        currentSet->samples=requestedSamples;
        
        dma_disable(DMA1, DMA_CH1);
        Timer2.setMode(CAPTURE_TIMER_CHANNEL,TIMER_DISABLED);
        Timer2.pause();
        captureState=Capture_complete;
        captureComplete();
        return;
    }
    // Ask for next set of samples
    captureState=Capture_armed;
    dma_setup_transfer(DMA1, DMA_CH1, &ADC1->regs->DR, DMA_SIZE_32BITS, dmaOverSampleBuffer, DMA_SIZE_32BITS, (DMA_MINC_MODE | DMA_TRNS_CMPLT));// Receive buffer DMA
    dma_set_num_transfers(DMA1, DMA_CH1, DMA_OVERSAMPLING_COUNT );
    dma_enable(DMA1, DMA_CH1); // Enable the channel and start the transfer.
}

// EOF

