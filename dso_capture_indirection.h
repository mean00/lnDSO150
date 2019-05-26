
typedef struct 
{
    void                        (*stopCapture)(void);
    DSOCapture::DSO_TIME_BASE   (*getTimeBase)(void);
    bool                        (*prepareSampling)(void);
    const char *                (*getTimeBaseAsText)();
    bool                        (*startCapture) (int count);
    
}CaptureFunctionTable;

const CaptureFunctionTable TimerTableTrigger=
{
    DSOCapture::stopCaptureTimer,
    DSOCapture::getTimeBaseTimer,
    DSOCapture::prepareSamplingTimer,
    DSOCapture::getTimeBaseAsTextTimer,
    DSOCapture::startCaptureTimerTrigger,
};
const CaptureFunctionTable DmaTableTrigger=
{
    DSOCapture::stopCaptureDma,
    DSOCapture::getTimeBaseDma,
    DSOCapture::prepareSamplingDma,
    DSOCapture::getTimeBaseAsTextDma,
    DSOCapture::startCaptureDmaTrigger,
};

const CaptureFunctionTable TimerTableRunning=
{
    DSOCapture::stopCaptureTimer,
    DSOCapture::getTimeBaseTimer,
    DSOCapture::prepareSamplingTimer,
    DSOCapture::getTimeBaseAsTextTimer,
    DSOCapture::startCaptureDma,
};
const CaptureFunctionTable DmaTableRunning=
{
    DSOCapture::stopCaptureDma,
    DSOCapture::getTimeBaseDma,
    DSOCapture::prepareSamplingDma,
    DSOCapture::getTimeBaseAsTextDma,
    DSOCapture::startCaptureTimer,
};

const CaptureFunctionTable *currentTable=&TimerTableTrigger;


/**
 * 
 * @param count
 * @return 
 */
bool     DSOCapture::prepareSampling ()
{
  return currentTable->prepareSampling();
}
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
 * @param timeBase
 * @return 
 */

bool     DSOCapture::setTimeBase(DSOCapture::DSO_TIME_BASE timeBase)
{
    if(timeBase>DSO_TIME_BASE_MAX)
    {
        xAssert(0);
    }
    if(timeBase<DSO_TIME_BASE::DSO_TIME_BASE_5MS) // fast mode
    {
        captureFast   =true;
        currentTimeBase=timeBase;
        currentTable=&DmaTableTrigger;
        
    }else
    {
        captureFast=false;
        currentTable=&TimerTableTrigger;
        currentTimeBase=timeBase-DSO_TIME_BASE::DSO_TIME_BASE_5MS;
    }
}
/**
 * 
 * @return 
 */
DSOCapture::DSO_TIME_BASE DSOCapture::getTimeBaseTimer()
{
    return (DSOCapture::DSO_TIME_BASE)(currentTimeBase+DSO_TIME_BASE::DSO_TIME_BASE_5MS);
}
DSOCapture::DSO_TIME_BASE DSOCapture::getTimeBaseDma()
{
    return (DSOCapture::DSO_TIME_BASE)currentTimeBase;
}
DSOCapture::DSO_TIME_BASE DSOCapture::getTimeBase()
{
    return currentTable->getTimeBase();        
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
 */
void DSOCapture::stopCaptureTimer()
{
    adc->stopTimeCapture();     
}


/**
 * 
 * @return 
 */
const char *DSOCapture::getTimeBaseAsText()
{
   return currentTable->getTimeBaseAsText();        
}

//--
const char *DSOCapture::getTimeBaseAsTextDma()
{
   return tSettings[currentTimeBase].name;
}
const char *DSOCapture::getTimeBaseAsTextTimer()
{
    return timerBases[currentTimeBase].name;
}

//------------------------------------------------------


bool       DSOCapture:: startCapture (int count)
{
  triggerValueADC=voltToADCValue(triggerValueFloat);
  controlButtons->updateCouplingState();
  return currentTable->startCapture(count);
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
