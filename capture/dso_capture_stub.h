
#pragma once

typedef void (captureCb)();
/**
 */
class demoCapture
{
public:
    
             demoCapture(lnPin pin);
        void setCb(captureCb *cb);
        bool getData(int &nb, float *f);
        bool startCapture(int nb);
protected:
    lnPin _pin;
    captureCb *_cb;
    int     _nb;
    
    
};