/***************************************************
  DSO-stm32duino test signal driver
 * (c) mean 2019 fixounet@free.fr
 ****************************************************/

#include <Wire.h>
#include "SPI.h"
#include "dso_test_signal.h"
#include "helpers/helper_pwm.h"
/**
 */
testSignal::testSignal(int pin,int pinAmp)
{
    this->pinAmp=pinAmp;
    this->pinSignal=pin;  
    digitalWrite(pin,1);
    pinMode(pin,PWM); 
    setAmplitude(true);
    setFrequency(1000);  
}
/**
 */
 bool testSignal::setFrequency(int fq)
 {
    setPWMPinFrequency(pinSignal, fq);
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
           pinMode(pinAmp,INPUT_FLOATING);
     }else
     {
          pinMode(pinAmp,OUTPUT_OPEN_DRAIN);
          digitalWrite(pinAmp,0);
     }
     return true;
 }
 
 // EOF
