
/**
 * 
 * @param in
 * @param out
 * @param count
 * @param set
 * @param expand
 * @param stats
 * @param triggerValue
 * @param mode
 * @return 
 */
static int transformTimer(int16_t *in, float *out,int count, VoltageSettings *set,int expand,CaptureStats &stats)
{
   if(!count) return false;
   stats.xmin=200;
   stats.xmax=-200;
   stats.avg=0;
   int ocount=(count*4096)/expand;
   if(ocount>240)
   {
       ocount=240;
   }
   ocount&=0xffe;
   int dex=0;
   
   // First
   float f;
       f=(float)in[0]; 
       f-=set->offset;
       f*=set->multiplier;       
       if(f>stats.xmax) stats.xmax=f;
       if(f<stats.xmin) stats.xmin=f;       
       out[0]=f; // Unit is now in volt
       stats.avg+=f;
       dex+=expand;
   
    for(int i=1;i<ocount;i++)
    {

        f=*(in+2*(dex/4096));
        f-=set->offset;
        f*=set->multiplier;
        if(f>stats.xmax) stats.xmax=f;
        if(f<stats.xmin) stats.xmin=f;       
        out[i]=f; // Unit is now in volt
        stats.avg+=f;
        dex+=expand;
    }   
   stats.avg/=count;
   return ocount;
}

/**
 * 
 * @return 
 */
bool DSOCapturePriv::prepareSamplingTimer()
{
    return adc->prepareTimerSampling(timerBases[currentTimeBase].fq);
}

/**
 * 
 * @return 
 */
DSOCapture::DSO_TIME_BASE DSOCapturePriv::getTimeBaseTimer()
{
    return (DSOCapture::DSO_TIME_BASE)(currentTimeBase+DSO_TIME_BASE::DSO_TIME_BASE_5MS);
}



/**
 * 
 */
void DSOCapturePriv::stopCaptureTimer()
{
    adc->stopTimeCapture();     
}


/**
 * 
 * @return 
 */

const char *DSOCapturePriv::getTimeBaseAsTextTimer()
{
    return timerBases[currentTimeBase].name;
}

/**
 * 
 * @param count
 * @return 
 */
bool       DSOCapturePriv:: startCaptureTimer (int count)
{    
    return adc->startTimerSampling(count);
}
/**
  * 
 */
bool       DSOCapturePriv:: startCaptureTimerTrigger (int count)
{
    return adc->startTriggeredTimerSampling(count,triggerValueADC);
}
/**
 */
bool DSOCapturePriv::taskletTimer()
{    
    FullSampleSet fset; // Shallow copy
    int16_t *p;

    int currentVolt=currentVoltageRange; // use a local copy so that it does not change in the middle
    
    if(!adc->getSamples(fset))
        return false;
    if(!fset.set1.samples)
    {
        return false;
    }

    CapturedSet *set=captureSet;        
    int scale=vSettings[currentVolt].inputGain;
    int expand=4096;

    float *data=set->data;    
    if(fset.shifted)
        p=((int16_t *)fset.set1.data)+1;
    else
        p=((int16_t *)fset.set1.data);

    set->samples=transformTimer(
                                    p,
                                    data,
                                    fset.set1.samples,
                                    vSettings+currentVolt,
                                    expand,
                                    set->stats);
    if(fset.set2.samples)
    {
        CaptureStats otherStats;
        if(fset.shifted)
            p=((int16_t *)fset.set2.data)+1;
        else
            p=((int16_t *)fset.set2.data);
        int sample2=transformTimer(
                                    p,
                                    data+set->samples,
                                    fset.set2.samples,
                                    vSettings+currentVolt,
                                    expand,
                                    otherStats);                
        set->stats.avg= (set->stats.avg*set->samples+otherStats.avg*fset.set2.samples)/(set->samples+fset.set2.samples);
        set->samples+=sample2;
        if(otherStats.xmax>set->stats.xmax) set->stats.xmax=otherStats.xmax;
        if(otherStats.xmin<set->stats.xmin) set->stats.xmin=otherStats.xmin;

    }
    set->stats.frequency=-1;

    float f=computeFrequency(fset.shifted,fset.set1.samples,fset.set1.data);
    f=((float)timerBases[currentTimeBase].fq)*1000./f;
    set->stats.frequency=f;
    set->stats.trigger=120;

    // Data ready!
    captureSemaphore->give();
    return true;
}