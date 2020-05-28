
#include "Arduino.h"
#include "MapleFreeRTOS1000_pp.h"
/**
 * 
 * @param pin
 * @param frequency
 * @return 
 */
bool setPWMPinFrequency(int pin, int frequency)
{
  
    HardwareTimer *timer;
    
    int channel=PIN_MAP[pin].timer_channel;
    timer_dev *dev=PIN_MAP[pin].timer_device;
    if(!dev) return false;
    if(dev==&timer1)    timer=&Timer1;
    else
        if(dev==&timer2)    timer=&Timer2;
        else
            if(dev==&timer3)    timer=&Timer3;
            else
                xAssert(0);
    timer->pause();
    timer->setPrescaleFactor(F_CPU/4000000); // we aim at 4 Mhz base clock
    int v=(4000000/frequency) &~1;
    timer->setCount(0);
    timer->setOverflow(v);
    timer->setCompare(channel,v/2); // square output
    timer->refresh();
    timer->resume();
    return true;
}