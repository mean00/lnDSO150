/***************************************************
 STM32 duino based firmware for DSO SHELL/150
 *  * GPL v2
 * (c) mean 2019 fixounet@free.fr
 ****************************************************/
#include "dso_global.h"
#include "dso_adc.h"
#include "dso_capture.h"
#include "dso_capture_priv.h"

static int      currenTimeBase=0;
static int      currentVoltageRange=0;
static bool     captureFast=true;
extern DSOADC   *adc;

extern int transform(int32_t *bfer, float *out,int count, VoltageSettings *set,int expand,float &xmin,float &xmax,float &avg);

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
bool     DSOCapture::setVoltageRange(DSOCapture::DSO_VOLTAGE_RANGE voltRange)
{
    currentVoltageRange=voltRange;
    return true;
}
/**
 * 
 * @return 
 */
DSOCapture::DSO_VOLTAGE_RANGE DSOCapture::getVoltageRange()
{
     return (DSOCapture::DSO_VOLTAGE_RANGE )currentVoltageRange;
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
        return adc->initiateSampling(ex/4096);
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

/**
 * 
 * @param count
 * @return 
 */
int DSOCapture::oneShotCapture(int count,float *outbuffer)
{
    int available;
    initiateSampling(count);
    uint32_t *buffer=    getSamples(available);
    
    int scale=vSettings[currentVoltageRange].inputGain;
    float xmin,xmax,avg;    
    count=transform((int32_t *)buffer,outbuffer,available,vSettings+currentVoltageRange,tSettings[currenTimeBase].expand4096,xmin,xmax,avg);
    
    reclaimSamples(buffer);
    return count;
}
/**
 * 
 * @param count
 * @param samples
 * @param waveForm
 * @return 
 */
bool DSOCapture::captureToDisplay(int count,float *samples,uint8_t *waveForm)
{
    float gain=vSettings[currentVoltageRange].displayGain;
    for(int j=0;j<count;j++)
        {
            float v=samples[j];
            v*=gain;
            v+=120;
            if(v>239) v=239;
            if(v<0) v=0;
            v=239-v;
            waveForm[j]=(uint8_t)v;
        }
    return true;
}
/**
 * 
 * @return 
 */
const char *DSOCapture::getTimeBaseAsText()
{
    if(captureFast)
    {
        return tSettings[currenTimeBase].name;
    }
    return timerBases[currenTimeBase].name;
}
/**
 * 
 * @return 
 */
const char *DSOCapture::getVoltageRangeAsText()
{
    return vSettings[currentVoltageRange].name;
}
