
#pragma once

/**
 */
class lnTimer;

class DSO_testSignal
{
public:
                 DSO_testSignal(lnPin pin,lnPin pinAmp);
            bool setFrequency(int fq);
            int  getFrequency();
            bool setAmplitude(bool large);
            bool getAmplitude();
            
protected:
            lnPin            pinAmp;
            lnPin            pinSignal;
            lnTimer        *_timer;
            int             _fq;
            bool            _large;
            
};
