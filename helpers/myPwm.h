
#include "MapleFreeRTOS1000_pp.h"
#pragma once

void myPwm(int pin,int fq); // short cut for pwmGetScaleOverFlow+pwmFromScalerAndOverflow 
void pwmPause(int pin);
void pwmRestart(int pin);
void pwmFromScalerAndOverflow(int pin, int scaler, int overFlow);
void pwmFromScalerAndOverflow(HardwareTimer *t,int channel, int scaler, int overFlow);
void pwmGetScaleOverFlow(int fq,int &scaler, int &ovf);
void pwmGetFrequency(int scaler, int ovf,int &fq);

HardwareTimer *pinToTimer(timer_dev *tdev);
class pwmQuickRestart
{
public:    
    pwmQuickRestart(int pin)
    {
        tdev=PIN_MAP[pin].timer_device;
        t=pinToTimer(tdev);
        timer_set_count(tdev,0);
        timer_generate_update(tdev);
    }
    void go(void)
    {
        timer_resume(tdev);
    }
protected:
    HardwareTimer *t;
    timer_dev *tdev;
};

