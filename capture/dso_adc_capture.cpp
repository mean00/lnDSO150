/***************************************************
 STM32 duino based firmware for DSO SHELL/150
 *  * GPL v2
 * (c) mean 2019 fixounet@free.fr
 ****************************************************/
#include "lnArduino.h"

#include "dso_adc_capture.h"
#include "dso_adc.h"

lnPin           DSOCapture::_pin;
captureCb       *DSOCapture::_cb;
int             DSOCapture::_nb;
int             DSOCapture::currentVoltageRange=0;
DSOCapture::DSO_TIME_BASE  DSOCapture::currentTimeBase=DSOCapture::DSO_TIME_BASE_1MS;
lnDSOAdc *_adc;
uint16_t internalAdcBuffer[1024];

/**
 * 
 * @param voltRange
 * @return 
 */
bool     DSOCapture::setVoltageRange(DSOCapture::DSO_VOLTAGE_RANGE voltRange)
{
    DSOCapture::currentVoltageRange=voltRange;
    DSOInputGain::setGainRange(vSettings[DSOCapture::currentVoltageRange].gain);
    return true;
}
/**
 * 
 * @return 
 */
const char * DSOCapture::getVoltageRangeAsText()
{
    return  vSettings[DSOCapture::currentVoltageRange].name;
}

/**
 * 
 * @return 
 */
DSOCapture::DSO_VOLTAGE_RANGE DSOCapture::getVoltageRange()
{
     return (DSOCapture::DSO_VOLTAGE_RANGE )DSOCapture::currentVoltageRange;
}

/**
 * 
 * @param timeBase
 */
void            DSOCapture::setTimeBase(DSO_TIME_BASE timeBase)
{    
    currentTimeBase=timeBase;
    _adc->setSource(3,3,timerBases[currentTimeBase].fq,_pin);
    Logger("New timebase=%d : %s, fq=%d\n",(int)timeBase,timerBases[timeBase].name,timerBases[timeBase].fq);
    _adc->setSmpt(timerBases[currentTimeBase].rate);
}
/**
 * 
 * @return 
 */
DSOCapture::DSO_TIME_BASE   DSOCapture::getTimeBase()
{
    return currentTimeBase;
}
/**
 * 
 * @return 
 */
const char *    DSOCapture::getTimeBaseAsText()
{
    return timerBases[currentTimeBase].name;
}



void DSOCapture::initialize(lnPin pin)
{
    _pin=pin;
    _adc=new lnDSOAdc(0);
    _adc->setSource(3,3,1000,_pin);
}

static void captureDone(int n)
{
    
    DSOCapture::captureDone(n);
}
void DSOCapture::setCb(captureCb *cb)
{
     _cb=cb;
}
/**
 * 
 */
void DSOCapture::captureDone(int nb)
{
    xAssert(_cb);
    _cb( );
}
/**
 * 
 * @param nb
 * @return 
 */
bool DSOCapture::startCapture(int nb)
{
    _nb=nb;
    _adc->setCb(captureDone);
    return _adc->startDmaTransfer(nb,internalAdcBuffer);
    return true;
}
/**
 * 
 * @param nb
 * @param f
 * @return 
 */
bool DSOCapture::getData(int &nb, float *f)
{
    _adc->endCapture();
    nb=_nb;
    for(int i=0;i<_nb;i++)
    {
        f[i]=(float)internalAdcBuffer[i];
    }
    return true;       
 }

// EOF
