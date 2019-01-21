/***************************************************
  DSO-stm32duino test signal driver
 * (c) mean 2019 fixounet@free.fr
 ****************************************************/

#include <Wire.h>
#include "SPI.h"
#include "testSignal.h"
/**
 */
testSignal::testSignal(int pin,int pinAmp,int timer, int channel)
{
    this->pinAmp=pinAmp;
    this->channel=channel;
    pwmtimer=new HardwareTimer(timer);
    digitalWrite(pin,1);
    pinMode(pin,PWM); 
    setAmplitute(true);
    setFrequency(1000);
}
/**
 */
 bool testSignal::setFrequency(int fq)
 {
    pwmtimer->pause();
    pwmtimer->setPrescaleFactor(18);
    //  4Mhz tick
    int v=(4000000/fq)&~1;
    pwmtimer->setCount(0);
    pwmtimer->setOverflow(v);
    pwmtimer->setCompare(channel, v/2);  
    pwmtimer->refresh();
    pwmtimer->resume();
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
 bool testSignal::setAmplitute(bool  large)
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
