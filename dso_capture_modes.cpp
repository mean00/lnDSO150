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
};
/**
 */
const CaptureFunctionTable DmaTableRunning=
{
    DSOCapturePriv::stopCaptureDma,
    DSOCapturePriv::getTimeBaseDma,
    DSOCapturePriv::prepareSamplingDma,
    DSOCapturePriv::getTimeBaseAsTextDma,
    DSOCapturePriv::startCaptureTimer,
    DSOCapturePriv::taskletDma,
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
    if(timeBase>DSO_TIME_BASE_MAX)
    {
        xAssert(0);
    }
    if(timeBase<DSO_TIME_BASE::DSO_TIME_BASE_5MS) // fast mode
    {
        DSOCapturePriv::currentTimeBase=timeBase;
        currentTable=&DmaTableTrigger;
        
    }else
    {
        currentTable=&TimerTableTrigger;
        DSOCapturePriv::currentTimeBase=timeBase-DSO_TIME_BASE::DSO_TIME_BASE_5MS;
    }
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
bool       DSOCapture:: startCapture (int count)
{
  DSOCapturePriv::triggerValueADC=DSOCapturePriv::voltToADCValue(DSOCapturePriv::triggerValueFloat);
  controlButtons->updateCouplingState();
  return currentTable->startCapture(count);
}

// EOF