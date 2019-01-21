/***************************************************
 
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
  */
 bool testSignal::setAmplitute(bool  large)
 {
#if 0

    if(0) /// 3.3 v
    {
        pinMode(PA12,INPUT_PULLUP);
    }else
    {
        pinMode(PA12,OUTPUT);
        digitalWrite(PA12,0);
    }
    
#if 0
        3.3 V => PA12 = intput + pullup
        pa7=output, no pullup
                
        0.1V => PA12=> output, no pullup, output=0              
#endif
    
#endif     
     return true;
 }
 
 // EOF
