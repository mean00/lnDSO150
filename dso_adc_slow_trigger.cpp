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
    FancyInterrupts::disable();
    timerRead=timerWrite=0;
    ADC_TIMER.attachInterrupt(ADC_TIMER_CHANNEL, Timer_Trigger_Event);
    captureState=Capture_armed;
    startInternalDmaSampling();   
    
    ADC_TIMER.setCompare(ADC_TIMER_CHANNEL, ADC_TIMER_COUNT);    
    ADC_TIMER.setMode(ADC_TIMER_CHANNEL, TIMER_OUTPUTCOMPARE); // start timer
    ADC_TIMER.refresh();
    ADC_TIMER.resume();
    
    FancyInterrupts::enable();
    
} 
void DSOADC::Timer_Trigger_Event() 
{    
    nbTimer++;
    instance->timerTriggerCapture();
}

#define NEXT_TRANSFER()  {   captureState=Capture_armed;     nextAdcDmaTransfer(DMA_OVERSAMPLING_COUNT,dmaOverSampleBuffer);}

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

    if(timerWrite < requestedSamples/2) // not enough samples
    {
        oldTriggerValue=avg2;
        NEXT_TRANSFER();
        return;     
    }
    // enough preloaded ?
    if(!slowTriggered )
    {
        uint32_t copy=oldTriggerValue;
        oldTriggerValue=avg2;
        //  Trigger match
        bool down=false,up=false;
        if(copy>slowTriggerValue && avg2<=slowTriggerValue)
        {
            down=true;
        }
        if(copy<slowTriggerValue && avg2>=slowTriggerValue)
        {
            up=true;
        }
        
        if(((_triggerMode!=Trigger_Rising )&& down) || ((_triggerMode!=Trigger_Falling) && up))
        {
            slowTriggered=true;
            timerRead=timerWrite-(requestedSamples/2);            
            NEXT_TRANSFER();
            return;            
        }                
    }
  
    if(!slowTriggered || ((timerWrite-timerRead)<requestedSamples))
    {
        NEXT_TRANSFER();
        return;
    }
    // Ok, triggered + got enough samples, we can stop here
    dma_disable(DMA1, DMA_CH1);
    ADC_TIMER.setMode(ADC_TIMER_CHANNEL,TIMER_DISABLED);
    ADC_TIMER.pause();
    captureState=Capture_complete;
    uint16_t  *source=adcInternalBuffer+(timerRead%ADC_INTERNAL_BUFFER_SIZE);
    uint16_t  *end=adcInternalBuffer+ADC_INTERNAL_BUFFER_SIZE;


    SampleSet one,two;
    int len=timerWrite-timerRead;
    if((timerWrite%ADC_INTERNAL_BUFFER_SIZE)>(timerRead%ADC_INTERNAL_BUFFER_SIZE))
    {
        one.set(len,adcInternalBuffer+(timerRead%ADC_INTERNAL_BUFFER_SIZE));
        two.set(0,NULL);
    }else
    {
        int left=ADC_INTERNAL_BUFFER_SIZE-(timerRead%ADC_INTERNAL_BUFFER_SIZE);

        one.set(len,adcInternalBuffer+(timerRead%ADC_INTERNAL_BUFFER_SIZE));
        two.set(len-left,adcInternalBuffer);            
    }
    captureComplete(one,two);
}

// EOF

