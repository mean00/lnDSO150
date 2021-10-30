/***************************************************
 STM32 duino based firmware for DSO SHELL/150
 *  * GPL v2
 * (c) mean 2019 fixounet@free.fr
 ****************************************************/
#include "lnArduino.h"
#include "lnCpuID.h"
#include "dso_capture.h"
#include "dso_adc.h"

lnPin           DSOCapture::_pin;
captureCb      *DSOCapture::_cb;
int             DSOCapture::_nb;
int             DSOCapture::currentVoltageRange=0;
DSOCapture::DSO_TIME_BASE  DSOCapture::currentTimeBase=DSOCapture::DSO_TIME_BASE_1MS;
lnDSOAdc *DSOCapture::_adc;
uint16_t *internalAdcBuffer;
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
    xAssert(timerBases[currentTimeBase].timeBase==timerADC[currentTimeBase].timeBase);    
    _adc->setSource(timerBases[currentTimeBase].fq,_pin,timerADC[currentTimeBase].scale,timerADC[currentTimeBase].rate,timerADC[currentTimeBase].overSampling);
    Logger("New timebase=%d : %s, fq=%d\n",(int)timeBase,timerBases[timeBase].name,timerBases[timeBase].fq);    
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

const int cycles[8]={ 14,20,26,41,54,68,84,252};

static int lin2log2(int in)
{
    int out=0;
    while(in>(1<<(out+1)) && out<8) out++;
    return out;
}
#define THRESHOLD_TO_PREFER_OVERSAMPLING 84
void DSOCapture::initialize(lnPin pin)
{
    internalAdcBuffer=new uint16_t[1024];
    _state=CAPTURE_STOPPED;
    _pin=pin;
    
    // Use max # of cycles possible
    int adcInputClock=lnPeripherals::getClock(pADC0)/2;
    int maxTimerFq=lnPeripherals::getClock(pTIMER3)/4;
    bool hasOverSampling=0+1*(lnCpuID::vendor()==lnCpuID::LN_MCU_GD32);
    for(int i=0;i<DSO_NB_TIMESCALE;i++)
    {
        timerADC[i].scale=lnADC_CLOCK_DIV_BY_2;        
        int samplingFq=timerBases[i].fq;        
        int r=adcInputClock/samplingFq;
        int overSampling=0;
        
        if(hasOverSampling && r>=2*THRESHOLD_TO_PREFER_OVERSAMPLING)
        {
            overSampling=lin2log2(r/THRESHOLD_TO_PREFER_OVERSAMPLING);
            // make sure we done exceed max Timer FQ
            while(((samplingFq<<overSampling)>maxTimerFq) && overSampling) overSampling--;
            r>>=overSampling;
        }
        timerADC[i].overSampling=overSampling;
        Logger("%d: R=%d overSampling=%d \n",i,r,overSampling);
        // Lookup up the biggest cycle that fits the frequency we need
        // the higher it is, the more accurate the ADC is (?)
        for(int c=7;c>=0;c--)
        {
            if(r>cycles[c]) 
            {
                timerADC[i].rate=(lnADC_CYCLES)c;
                break;
            }                
        }
        
    }    
    _adc=new lnDSOAdc(0,3,3);     // timer 3 channel 3
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
