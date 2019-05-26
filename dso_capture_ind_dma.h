
/**
 * 
 * @return 
 */
bool DSOCapture::prepareSamplingDma()
{
  //     
    const TimeSettings *set= tSettings+currentTimeBase;
    return adc->prepareDMASampling(set->rate,set->prescaler);
}
DSOCapture::DSO_TIME_BASE DSOCapture::getTimeBaseDma()
{
    return (DSOCapture::DSO_TIME_BASE)currentTimeBase;
}


/**
 * 
 */
void DSOCapture::stopCaptureDma()
{
    
    adc->stopDmaCapture();
    
}
/**
 * 
 * @return 
 */
const char *DSOCapture::getTimeBaseAsTextDma()
{
   return tSettings[currentTimeBase].name;
}


/**
 * 
 * @param count
 * @return 
 */
bool       DSOCapture:: startCaptureDma (int count)
{
    int ex=count*tSettings[currentTimeBase].expand4096;
    return adc->startDMASampling(ex);
}
/**
 * 
 * @param count
 * @return 
 */
bool       DSOCapture:: startCaptureDmaTrigger (int count)
{
    
    int ex=count;
    ex=count*tSettings[currentTimeBase].expand4096;
    lastRequested=ex/4096;
    return adc->startDMATriggeredSampling(ex,triggerValueADC);
}

/**
 * 
 * @return 
 */
bool DSOCapture::taskletDma()
{
    xDelay(20);
    FullSampleSet fset; // Shallow copy
    int16_t *p;
    while(1)
    {
        int currentVolt=currentVoltageRange; // use a local copy so that it does not change in the middle
        int currentTime=currentTimeBase;
        bool findTrigger=false;
        if(!adc->getSamples(fset))
            continue;
        
        CapturedSet *set=captureSet;
        set->stats.trigger=-1;

        
        if(captureFast)   
        {
            findTrigger=refineCapture(fset);
            if(findTrigger)
                set->stats.trigger=120; // right in the middle
        }
    
        
        if(!fset.set1.samples)
        {
            continue;
        }
        int scale=vSettings[currentVolt].inputGain;
        int expand=4096;
        if(captureFast)            
        {
            expand=tSettings[currentTime].expand4096;
        }
        float *data=set->data;    
        if(fset.shifted)
            p=((int16_t *)fset.set1.data)+1;
        else
            p=((int16_t *)fset.set1.data);

        set->samples=transform(
                                        p,
                                        data,
                                        fset.set1.samples,
                                        vSettings+currentVolt,
                                        expand,
                                        set->stats,
                                        triggerValueFloat,
                                        adc->getTriggerMode());
        if(fset.set2.samples)
        {
            CaptureStats otherStats;
            if(fset.shifted)
                p=((int16_t *)fset.set2.data)+1;
            else
                p=((int16_t *)fset.set2.data);
            int sample2=transform(
                                        p,
                                        data+set->samples,
                                        fset.set2.samples,
                                        vSettings+currentVolt,
                                        expand,
                                        otherStats,
                                        triggerValueFloat,
                                        adc->getTriggerMode());                
            if(set->stats.trigger==-1 && otherStats.trigger!=-1) 
            {
                set->stats.trigger=otherStats.trigger+set->samples;
            }            
            set->stats.avg= (set->stats.avg*set->samples+otherStats.avg*fset.set2.samples)/(set->samples+fset.set2.samples);
            set->samples+=sample2;
            if(otherStats.xmax>set->stats.xmax) set->stats.xmax=otherStats.xmax;
            if(otherStats.xmin<set->stats.xmin) set->stats.xmin=otherStats.xmin;
            
        }
        set->stats.frequency=-1;
        
        float f=computeFrequency(fset.shifted,fset.set1.samples,fset.set1.data);
        if(captureFast)
        {
            f=(float)(tSettings[currentTimeBase].fqInHz)*1000./f;
        }else
        {
            f=((float)timerBases[currentTimeBase].fq)*1000./f;
        }
        set->stats.frequency=f;
        
        // Data ready!
        captureSemaphore->give();
    }
}