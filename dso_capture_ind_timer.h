
/**
 * 
 * @return 
 */
bool DSOCapture::prepareSamplingTimer()
{
    return adc->prepareTimerSampling(timerBases[currentTimeBase].fq);
}

/**
 * 
 * @return 
 */
DSOCapture::DSO_TIME_BASE DSOCapture::getTimeBaseTimer()
{
    return (DSOCapture::DSO_TIME_BASE)(currentTimeBase+DSO_TIME_BASE::DSO_TIME_BASE_5MS);
}



/**
 * 
 */
void DSOCapture::stopCaptureTimer()
{
    adc->stopTimeCapture();     
}


/**
 * 
 * @return 
 */

const char *DSOCapture::getTimeBaseAsTextTimer()
{
    return timerBases[currentTimeBase].name;
}

/**
 * 
 * @param count
 * @return 
 */
bool       DSOCapture:: startCaptureTimer (int count)
{    
    return adc->startTimerSampling(count);
}
/**
  * 
 */
bool       DSOCapture:: startCaptureTimerTrigger (int count)
{
    return adc->startTriggeredTimerSampling(count,triggerValueADC);
}
/**
 */
bool DSOCapture::taskletTimer()
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