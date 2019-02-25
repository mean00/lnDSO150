#include "dso_global.h"
#include "dso_adc.h"
#include "dso_capture.h"
#include "dso_capture_priv.h"

static int currenTimeBase=0;
static int currentVoltageRange=0;
bool   captureFast=true;
extern DSOADC    *adc;
/**
 */


/**
 * 
 * @return 
 */
DSOCapture::DSO_TIME_BASE DSOCapture::getTimeBase()
{
    if(captureFast) 
        return (DSOCapture::DSO_TIME_BASE)currenTimeBase;
    return (DSOCapture::DSO_TIME_BASE)(currenTimeBase+DSO_TIME_BASE::DSO_TIME_BASE_5MS);
}
/**
 * 
 * @param voltRange
 * @return 
 */
bool     setVoltageRange(DSOCapture::DSO_VOLTAGE_RANGE voltRange)
{
    currentVoltageRange=voltRange;
    return true;
}
/**
 * 
 * @return 
 */
DSOCapture::DSO_VOLTAGE_RANGE getVoltageRange()
{
    return (DSOCapture::DSO_VOLTAGE_RANGE)currentVoltageRange;
}

/**
 * 
 * @param timeBase
 * @return 
 */
bool     DSOCapture::setTimeBase(DSOCapture::DSO_TIME_BASE timeBase)
{
    if(timeBase<DSO_TIME_BASE::DSO_TIME_BASE_5MS) // fast mode
    {
        captureFast   =true;
        currenTimeBase=timeBase;
        
    }else
    {
        captureFast=false;
        currenTimeBase=timeBase-DSO_TIME_BASE::DSO_TIME_BASE_5MS;
    }
}
/**
 * 
 * @param count
 * @return 
 */
bool     DSOCapture::initiateSampling (int count)
{
    if(captureFast)
    {
        //
        int ex=count*tSettings[currenTimeBase].expand4096;
        return adc->initiateSampling(count);
    }else
    {
        adc->setSlowMode(timerBases[currenTimeBase].fq);
        return adc->initiateTimerSampling(count);
    }
}
/**
 * 
 * @param count
 * @return 
 */
uint32_t *DSOCapture::getSamples(int &count)
{
    return adc->getSamples(count);
}
/**
 * 
 * @param buffer
 */
void     DSOCapture::reclaimSamples(uint32_t *buffer)
{
    adc->reclaimSamples(buffer);
}
