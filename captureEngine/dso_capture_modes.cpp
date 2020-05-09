/***************************************************
 STM32 duino based firmware for DSO SHELL/150
 *  * GPL v2
 * (c) mean 2019 fixounet@free.fr
 ****************************************************/
#include "dso_global.h"
#include "dso_adc.h"
#include "dso_capture.h"
#include "dso_capture_priv.h"

#include "DSO_config.h"
#include "stopWatch.h"
extern StopWatch watch;
/**
 */
const CaptureFunctionTable TimerTableTrigger=
{
    DSOCapturePriv::stopCaptureTimer,
    DSOCapturePriv::getTimeBaseTimer,
    DSOCapturePriv::prepareSamplingTimer,
    DSOCapturePriv::getTimeBaseAsTextTimer,
    DSOCapturePriv::startCaptureTimerTrigger,
    DSOCapturePriv::taskletTimer,
    DSOCapturePriv::nextCaptureTimerTrigger,
};
/**
 */
const CaptureFunctionTable DmaTableTrigger=
{
    DSOCapturePriv::stopCaptureDma,
    DSOCapturePriv::getTimeBaseDma,
    DSOCapturePriv::prepareSamplingDma,
    DSOCapturePriv::getTimeBaseAsTextDma,
    DSOCapturePriv::startCaptureDmaTrigger,
    DSOCapturePriv::taskletDma,
    DSOCapturePriv::nextCaptureDmaTrigger,
};
/**
 */
const CaptureFunctionTable TimerTableRunning=
{
    DSOCapturePriv::stopCaptureTimer,
    DSOCapturePriv::getTimeBaseTimer,
    DSOCapturePriv::prepareSamplingTimer,
    DSOCapturePriv::getTimeBaseAsTextTimer,
    DSOCapturePriv::startCaptureTimer,
    DSOCapturePriv::taskletTimer,
    DSOCapturePriv::nextCaptureTimer,
};
/**
 */
const CaptureFunctionTable DmaTableRunning=
{
    DSOCapturePriv::stopCaptureDma,
    DSOCapturePriv::getTimeBaseDma,
    DSOCapturePriv::prepareSamplingDma,
    DSOCapturePriv::getTimeBaseAsTextDma,
    DSOCapturePriv::startCaptureDma,
    DSOCapturePriv::taskletDmaRunning,
    DSOCapturePriv::nextCaptureDma,
};
/**
 */
const CaptureFunctionTable *currentTable=&TimerTableTrigger;

/**
 * 
 * @param count
 * @return 
 */
bool     DSOCapturePriv::prepareSampling ()
{
  return currentTable->prepareSampling();
}
/**
 * 
 * @param timeBase
 * @return 
 */

bool     DSOCapture::setTimeBase(DSOCapture::DSO_TIME_BASE timeBase)
{
    watch.ok();
    if(timeBase>DSO_TIME_BASE_MAX)
    {
        xAssert(0);
    }
    if(timeBase<=DSO_TIME_BASE::SLOWER_FAST_MODE) // fast mode
    {
        if(adc->getTriggerMode()!=DSOADC::Trigger_Run)
            currentTable=&DmaTableTrigger;
        else
            currentTable=&DmaTableRunning;
        DSOCapturePriv::currentTimeBase=timeBase;        
        
    }else
    {
        if(adc->getTriggerMode()!=DSOADC::Trigger_Run)
            currentTable=&TimerTableTrigger;
        else
            currentTable=&TimerTableRunning;                
        DSOCapturePriv::currentTimeBase=timeBase-DSO_TIME_BASE::SLOWER_FAST_MODE-1;
    }
    return true;
}
/**
 * 
 * @return 
 */
DSOCapture::DSO_TIME_BASE DSOCapture::getTimeBase()
{
    return currentTable->getTimeBase();        
}

/**
 * 
 * @return 
 */
const char *DSOCapture::getTimeBaseAsText()
{
   return currentTable->getTimeBaseAsText();        
}
/**
 * 
 * @param count
 * @return 
 */
bool       DSOCapturePriv:: startCapture (int count)
{
  DSOCapturePriv::triggerValueADC=DSOCapturePriv::voltToADCValue(DSOCapturePriv::triggerValueFloat);
  controlButtons->updateCouplingState();
  xAssert(DSOCapturePriv::taskletMode==DSOCapturePriv::Tasklet_Idle);
  
  // clear semaphore if needed
  adc->clearSemaphore();
  captureSemaphore->reset();
  
  DSOCapturePriv::taskletMode=DSOCapturePriv::Tasklet_Running;
  return currentTable->startCapture(count);
}

// EOF