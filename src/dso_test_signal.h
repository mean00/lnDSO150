
#pragma once

/**
 */
class lnTimer;

class DSO_testSignal
{
public:
                 DSO_testSignal(lnPin pin,lnPin pinAmp);
            bool setFrequency(int fq);
            bool setAmplitude(bool large);
            
protected:
            int            pinAmp;
            int            pinSignal;
            lnTimer        *_timer;
            
};