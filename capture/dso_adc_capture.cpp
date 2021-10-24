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
DSOCapture::captureState DSOCapture::_state=DSOCapture::CAPTURE_STOPPED;
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
float DSOCapture::getVoltToPix()
{
     return  vSettings[DSOCapture::currentVoltageRange].displayGain;
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
    _adc->setSource(3,3,timerBases[currentTimeBase].fq,_pin,timerADC[currentTimeBase].scale,timerADC[currentTimeBase].rate);
    Logger("New timebase=%d : %s, fq=%d\n",(int)timeBase,timerBases[timeBase].name,timerBases[timeBase].fq);
    _adc->setSmpt(timerADC[currentTimeBase].rate);
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
/**
 * 
 * @param pin
 */
void DSOCapture::initialize(lnPin pin)
{
    _state=CAPTURE_STOPPED;
    _pin=pin;
    _adc=new lnDSOAdc(0);    
    xAssert(timerBases[currentTimeBase].timeBase==timerADC[currentTimeBase].timeBase);
    _adc->setSource(3,3,timerBases[currentTimeBase].fq,_pin,timerADC[currentTimeBase].scale,timerADC[currentTimeBase].rate);
    setTimeBase(DSO_TIME_BASE_20US);
}
/**
 * 
 * @param n
 */
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
    _state=CAPTURE_DONE;
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
    switch(_state)
    {
        case CAPTURE_RUNNING:
        case CAPTURE_DONE:
                _adc->stopCapture();
                break;
        case CAPTURE_STOPPED:
            break;
        default:
            break;
    }
    _adc->setCb(captureDone);
    _state=CAPTURE_RUNNING;
    return _adc->startDmaTransfer(nb,internalAdcBuffer);
}
/**
 * 
 */
void DSOCapture::stopCapture()
{
    switch(_state)
    {
        case CAPTURE_RUNNING:
        case CAPTURE_DONE:            
            _adc->stopCapture();
            _state=CAPTURE_STOPPED;
            break;
        default:
            break;
    }    
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
    int offset=DSOInputGain::getOffset(0);
    float multiplier=DSOInputGain::getMultiplier();
    for(int i=0;i<_nb;i++)
    {
        int fint=(int)internalAdcBuffer[i]-offset;
        float z=(float)fint*multiplier;
        f[i]=z; // now in volt
    }
    return true;       
 }

// EOF
