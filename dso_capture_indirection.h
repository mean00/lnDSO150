/**
 */
typedef struct 
{
    void                        (*stopCapture)(void);
    DSOCapture::DSO_TIME_BASE   (*getTimeBase)(void);
    bool                        (*prepareSampling)(void);
    const char *                (*getTimeBaseAsText)();
    bool                        (*startCapture) (int count);
    bool                        (*tasklet)();
}CaptureFunctionTable;
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

//#include "dso_capture_ind_dma.h"
#include "dso_capture_ind_timer.h"

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