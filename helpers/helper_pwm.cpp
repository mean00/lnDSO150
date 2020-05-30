
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

bool setTimerFrequency(HardwareTimer *timer, int channel, int periodOnPs, int periodOffPs)
{  
    timer->pause();
    
    float  period;
    
    period=periodOnPs+periodOffPs;
    period=period/1000000000.;     //in second            
    
    float fq=1./period;
    
    // count*scale=F_CPU/fq
    
    float ratio=(float)F_CPU/fq;
    
    int iratio=ratio*1024.;
    
    int scale=1;
    int count=iratio/1024;
    
    while(count>65535)
    {
        scale++;
        count=iratio/(1024*scale);
    }
    timer->setPrescaleFactor(scale);         
    count=(period*F_CPU)/(float)scale;
    
    
    timer->setCount(0);
    timer->setOverflow(count);
    timer->setCompare(channel,count/2); // square output for now
    timer->refresh();
    timer->resume();
    return true;
}

/**
 * 
 * @param timer
 * @param channel
 * @param frequency
 * @return 
 */
bool setTimerFrequency(HardwareTimer *timer, int channel, int frequency)
{  
/*    
    timer->setPeriod(1000000/frequency);
    return true;
 * */
    timer->pause();
    float fq=frequency;
        
    
    float ratio=(float)F_CPU/(float)fq;
    
    int iratio=ratio*1024.;
    
    int scale=1;
    int count=iratio/1024;
    
    while(count>65535)
    {
        scale++;
        count=iratio/(1024*scale);
    }
    timer->setPrescaleFactor(scale);         
    count=((int)(F_CPU/((float)scale*frequency))+1)&~1;
    timer->setCount(0);
    timer->setOverflow(count);
    timer->setCompare(channel,count/2); // square output for now
    timer->refresh();

    return true;
}


