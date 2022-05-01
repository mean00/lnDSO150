/***************************************************
  DSO-stm32duino test signal driver
 * (c) mean 2019 fixounet@free.fr
 ****************************************************/

#include "lnArduino.h"
#include "dso_test_signal.h"
#include "lnTimer.h"

/**
 */

DSO_testSignal::DSO_testSignal(lnPin pin,lnPin pinAmp)
{
    this->pinAmp=pinAmp;
    this->pinSignal=pin;  
    setAmplitude(true);
    lnPinMode(pinSignal,lnPWM);    
    _timer=new lnTimer(pinSignal);
    _timer->setPwmMode(512);
    _timer->enable();    
    _fq=1000;
    _large=true;
    setFrequency(_fq);
    
}
/**
 */
 bool DSO_testSignal::setFrequency(int fq)
 {     
     _fq=fq;
     _timer->disable();
     _timer->setPwmFrequency(fq);
     _timer->enable();
    return true;
}
 /**
  * 
  * @param fq
  * @return 
  */
 int DSO_testSignal::getFrequency(void)
 {
     return _fq;
 }
 /**
  * The actual output is connected like that
  *    PA7  --- R16
  *    PA12 --- R31 Ohm----> out
  * 
  *     So if PA12 is floating, out is the PA7 output @3.3v
  *     if PA12 is output to the ground, output is 3.3*R31/(R16+R31)= ~ 100 mv
  * 
  * 
  */
 bool DSO_testSignal::setAmplitude(bool  large)
 {
     _large=large;
#ifndef     USE_FNIRSI_BUTTON     
     if(large)
     {
           lnPinMode(pinAmp,lnINPUT_FLOATING);
     }else
     {
          lnPinMode(pinAmp,lnOUTPUT_OPEN_DRAIN);
          lnDigitalWrite(pinAmp,0);
     }
#endif     
     return true;
 }
 /**
  * 
  * @return 
  */
 bool DSO_testSignal::getAmplitude()
 {
     return _large;
 }
 // EOF
