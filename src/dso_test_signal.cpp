/***************************************************
  DSO-stm32duino test signal driver
 * (c) mean 2019 fixounet@free.fr
 ****************************************************/

#include "lnArduino.h"
#include "dso_test_signal.h"
#include "lnTimer.h"

/**
 */

testSignal::testSignal(lnPin pin,lnPin pinAmp)
{
    this->pinAmp=pinAmp;
    this->pinSignal=pin;  
    setAmplitude(true);
    lnPinMode(pinSignal,lnPWM);    
    _timer=new lnTimer(pinSignal);
    _timer->setPwmMode(512);
    _timer->enable();
    setFrequency(1000);
    
}
/**
 */
 bool testSignal::setFrequency(int fq)
 {     
     _timer->disable();
     _timer->setPwmFrequency(fq);
     _timer->enable();
    return true;
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
 bool testSignal::setAmplitude(bool  large)
 {
     if(large)
     {
           lnPinMode(pinAmp,lnINPUT_FLOATING);
     }else
     {
          lnPinMode(pinAmp,lnOUTPUT_OPEN_DRAIN);
          lnDigitalWrite(pinAmp,0);
     }
     return true;
 }
 
 // EOF
