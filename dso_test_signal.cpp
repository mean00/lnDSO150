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
#if 0
    timer_dev *dev=Timer1.c_dev();
    int channel;
    *bb_perip(&(dev->regs).gen->CCER, 4 * (channel - 1)) = 1; // Disable normal & enable complimentary & regular
    *bb_perip(&(dev->regs).gen->CCER, 4 * (channel - 1)+2) = 1;
    *bb_perip(&(dev->regs).gen->CCER, 4 * (channel - 1)+3) = *bb_perip(&(dev->regs).gen->CCER, 4 * (channel - 1)+1);
    Timer1.pause();
    Timer1.refresh();
    Timer1.resume();
#endif
}
/**
 */
 bool testSignal::setFrequency(int fq)
 {
    setPWMPinFrequency(&Timer1,1, fq);
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
