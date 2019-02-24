#include "dso_global.h"
#include "dso_adc.h"
#include "dso_capture.h"

static int currenTimeBase=0;
bool   captureFast=true;
extern DSOADC    *adc;
/**
 */
typedef struct TimerTimeBase
{
  DSOCapture::DSO_TIME_BASE timeBase;
  const char    *name;
  int           fq;  
};
/**
 */
static const TimerTimeBase timerBases[]
{
    { DSOCapture::DSO_TIME_BASE_5MS,    "5ms",  4800},
    { DSOCapture::DSO_TIME_BASE_10MS,   "10ms", 2400},
    { DSOCapture::DSO_TIME_BASE_50MS,   "50ms", 480},
    { DSOCapture::DSO_TIME_BASE_100MS,  "100ms",240},
    { DSOCapture::DSO_TIME_BASE_500MS,  "500ms",48},
    { DSOCapture::DSO_TIME_BASE_1S,     "1s",   24}    
};

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
