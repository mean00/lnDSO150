/**
 
 *  This is the slow capture mode
 * i.e. we setup n Samples acquisition through DMA
 * and a timer interrupt grabs the result 
 */
#define TIMER_BUFFER_SIZE 256
static uint32_t timerBuffer[TIMER_BUFFER_SIZE] __attribute__ ((aligned (8)));;


static bool slowTriggered;
uint32_t oldTriggerValue,slowTriggerValue=2100;
uint32_t timerRead,timerWrite;
/**
 * 
 * @param count
 * @param buffer
 * @return 
 */
bool DSOADC::startTriggeredTimerSampling (int count,uint32_t triggerADC)
{
    if(!capturedBuffers.empty())
        return true; // We have data !
    slowTriggerValue=triggerADC;
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
    timerRead=timerWrite=0;
    Timer2.attachInterrupt(CAPTURE_TIMER_CHANNEL, Timer2Trigger_Event);
    captureState=Capture_armed;
    startInternalDmaSampling();   
    
    Timer2.setCompare(CAPTURE_TIMER_CHANNEL, 16);    
    Timer2.setMode(CAPTURE_TIMER_CHANNEL, TIMER_OUTPUTCOMPARE); // start timer
    Timer2.refresh();
    Timer2.resume();
    
    interrupts();
} 
void DSOADC::Timer2Trigger_Event() 
{    
    nbTimer++;
    instance->timerTriggerCapture();
}

#define NEXT_TRANSFER() \ 
    captureState=Capture_armed; \
    dma_setup_transfer(DMA1, DMA_CH1, &ADC1->regs->DR, DMA_SIZE_32BITS, dmaOverSampleBuffer, DMA_SIZE_32BITS, (DMA_MINC_MODE | DMA_TRNS_CMPLT)); \
    dma_set_num_transfers(DMA1, DMA_CH1, DMA_OVERSAMPLING_COUNT ); \
    dma_enable(DMA1, DMA_CH1); // Enable the channel and start the transfer.


/**
 * \fn timerCapture
 * \brief this is one is called by a timer interrupt
 */
void DSOADC::timerTriggerCapture()
{    
    uint32_t avg2=0;
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
    {
        uint32_t val=dmaOverSampleBuffer[i]>>16;
        avg2+=val;        
    }    
    avg2=(avg2+DMA_OVERSAMPLING_COUNT/2+1)/DMA_OVERSAMPLING_COUNT;
    
    
    if(timerWrite>0x20000200 && timerRead > 0x20000200)
    {
        timerRead-= 0x20000000;
        timerWrite-=0x20000000;
    }
    int xindex=timerWrite%TIMER_BUFFER_SIZE;
    
    timerBuffer[xindex]=avg2;
    timerWrite++;

    // enough preloaded ?
    if(!slowTriggered && timerWrite >= requestedSamples/2)
    {
        uint32_t copy=oldTriggerValue;
        oldTriggerValue=avg2;
        if(copy>slowTriggerValue && avg2<slowTriggerValue)
        {
            NEXT_TRANSFER();
            slowTriggered=true;
            timerRead=timerWrite-(requestedSamples/2);
            return;
            
        }                
    }
  

    if(slowTriggered && (timerWrite-timerRead)>=requestedSamples)
    {
        currentSet->samples=requestedSamples;
        
        dma_disable(DMA1, DMA_CH1);
        Timer2.setMode(CAPTURE_TIMER_CHANNEL,TIMER_DISABLED);
        Timer2.pause();
        captureState=Capture_complete;
        for(int i=0;i<requestedSamples;i++)
        {
            currentSamplingBuffer[i]=timerBuffer[(timerRead+i)%TIMER_BUFFER_SIZE]<<16;
        }
        captureComplete();
        return;
    }
    NEXT_TRANSFER();
}

// EOF

