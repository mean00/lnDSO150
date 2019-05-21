
#pragma once
#include "HardwareTimer.h"
/**
 */
class testSignal
{
public:
                 testSignal(int pin,int pinAmp,int timer, int channel);
            bool setFrequency(int fq);
            bool setAmplitude(bool large);
            
protected:
            HardwareTimer *pwmtimer;
            int            pinAmp;
            int            channel;
            
};