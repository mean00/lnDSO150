#include "lnArduino.h"
#include "lnADC.h"
#include "dso_capture_stub.h"
#include "dso_capture_input.h"

lnTimingAdc *_adc;

uint16_t internalAdcBuffer[1024];


demoCapture::demoCapture(lnPin pin)
{
    _pin=pin;
    _adc=new lnTimingAdc(0);
    _adc->setSource(3,3,1000,1,&_pin);
}


void demoCapture::setCb(captureCb *cb)
{
     _cb=cb;
}
bool demoCapture::startCapture(int nb)
{
    _nb=nb;
     if(_adc->multiRead(nb,internalAdcBuffer))
     {
      _cb();    
      return true;
     }
    return false;
}
bool demoCapture::getData(int &nb, float *f)
{
    nb=_nb;
    for(int i=0;i<_nb;i++)
    {
        f[i]=(float)internalAdcBuffer[i];
    }
    return true;       
 }
