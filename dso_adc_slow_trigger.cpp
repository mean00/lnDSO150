/**
 
 *  This is the slow capture mode / triggered
 *  We'll take the capture only if the trigger happens & try to center it
 */


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
    // Reset settings
    slowTriggerValue=triggerADC;
    oldTriggerValue=triggerADC-1;
    slowTriggered=false;
    currentIndex=0;
    //
    if(count>ADC_INTERNAL_BUFFER_SIZE/2)
        count=ADC_INTERNAL_BUFFER_SIZE/2;
  
    requestedSamples=count;

    
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
    nextAdcDmaTransfer(DMA_OVERSAMPLING_COUNT,dmaOverSampleBuffer);

/**
 * \fn timerCapture
 * \brief this is one is called by a timer interrupt
 */
void DSOADC::timerTriggerCapture()
{    
    uint32_t avg2=0;
    if(! validateAverageSample(avg2))
        return;
    
    
    if(timerWrite>0x20000200 && timerRead > 0x20000200)
    {
        timerRead-= 0x20000000;
        timerWrite-=0x20000000;
    }
    int xindex=timerWrite%ADC_INTERNAL_BUFFER_SIZE;
    
    adcInternalBuffer[xindex]=avg2;
    timerWrite++;

    // enough preloaded ?
    if(!slowTriggered && timerWrite >= requestedSamples/2)
    {
        uint32_t copy=oldTriggerValue;
        oldTriggerValue=avg2;
        // -|_
        if(copy>slowTriggerValue && avg2<slowTriggerValue)
        {
            slowTriggered=true;
            timerRead=timerWrite-(requestedSamples/2);            
            NEXT_TRANSFER();
            return;            
        }                
    }
  
    // do we have enough sample ?
    if(slowTriggered && (timerWrite-timerRead)>=requestedSamples)
    {
        dma_disable(DMA1, DMA_CH1);
        Timer2.setMode(CAPTURE_TIMER_CHANNEL,TIMER_DISABLED);
        Timer2.pause();
        captureState=Capture_complete;
        uint32_t  *source=adcInternalBuffer+(timerRead%ADC_INTERNAL_BUFFER_SIZE);
        uint32_t  *end=adcInternalBuffer+ADC_INTERNAL_BUFFER_SIZE;
        
         
        SampleSet one,two;
        int len=timerWrite-timerRead;
        if((timerWrite%ADC_INTERNAL_BUFFER_SIZE)>(timerRead%ADC_INTERNAL_BUFFER_SIZE))
        {
            one.samples=len;
            one.data=adcInternalBuffer+(timerRead%ADC_INTERNAL_BUFFER_SIZE);
            two.samples=0;
            two.data=NULL;
        }else
        {
            int left=ADC_INTERNAL_BUFFER_SIZE-(timerRead%ADC_INTERNAL_BUFFER_SIZE);
            one.samples=len;
            one.data=adcInternalBuffer+(timerRead%ADC_INTERNAL_BUFFER_SIZE);
            two.samples=len-left;
            two.data=adcInternalBuffer;
        }
        captureComplete(false,one,two);

        return;
    }
    NEXT_TRANSFER();
}

// EOF

