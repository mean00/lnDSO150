
#pragma once
#include "HardwareTimer.h"
/**
 */
class testSignal
{
public:
                 testSignal(int pin,int pinAmp);
            bool setFrequency(int fq);
            bool setAmplitude(bool large);
            
protected:
            int            pinAmp;
            int            pinSignal;
            
};