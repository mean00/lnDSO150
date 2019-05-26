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
    DSOCapture::stopCaptureTimer,
    DSOCapture::getTimeBaseTimer,
    DSOCapture::prepareSamplingTimer,
    DSOCapture::getTimeBaseAsTextTimer,
    DSOCapture::startCaptureTimerTrigger,
    DSOCapture::taskletTimer,
};
/**
 */
const CaptureFunctionTable DmaTableTrigger=
{
    DSOCapture::stopCaptureDma,
    DSOCapture::getTimeBaseDma,
    DSOCapture::prepareSamplingDma,
    DSOCapture::getTimeBaseAsTextDma,
    DSOCapture::startCaptureDmaTrigger,
    DSOCapture::taskletDma,
};
/**
 */
const CaptureFunctionTable TimerTableRunning=
{
    DSOCapture::stopCaptureTimer,
    DSOCapture::getTimeBaseTimer,
    DSOCapture::prepareSamplingTimer,
    DSOCapture::getTimeBaseAsTextTimer,
    DSOCapture::startCaptureTimer,
    DSOCapture::taskletTimer,
};
/**
 */
const CaptureFunctionTable DmaTableRunning=
{
    DSOCapture::stopCaptureDma,
    DSOCapture::getTimeBaseDma,
    DSOCapture::prepareSamplingDma,
    DSOCapture::getTimeBaseAsTextDma,
    DSOCapture::startCaptureTimer,
    DSOCapture::taskletDma,
};
/**
 */
const CaptureFunctionTable *currentTable=&TimerTableTrigger;

#include "dso_capture_ind_dma.h"
#include "dso_capture_ind_timer.h"

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
  triggerValueADC=voltToADCValue(triggerValueFloat);
  controlButtons->updateCouplingState();
  return currentTable->startCapture(count);
}

// EOF