#include "Arduino.h"
#include "myPwm.h"

HardwareTimer *pinToTimer(timer_dev *tdev)
{    
    HardwareTimer *t=NULL;
    if(!tdev) xAssert(0);
    if(Timer1.c_dev()==tdev) t=&Timer1;
    if(Timer2.c_dev()==tdev) t=&Timer2;
    if(Timer3.c_dev()==tdev) t=&Timer3;
    if(Timer4.c_dev()==tdev) t=&Timer4;
    
    if(!t) xAssert(0);
    return t;
}

/**
 * 
 * @param scaler
 * @param ovf
 * @param fq
 */
void pwmGetFrequency(int scaler, int ovf,int &fq)
{
    fq=F_CPU/(scaler*ovf);
}
/**
 * 
 * @param fq
 * @param scaler
 * @param ovf
 */
void pwmGetScaleOverFlow(int fq,int &scaler, int &ovf)
{
    scaler=F_CPU/(fq*65535);
    scaler+=1;
    int high=F_CPU/scaler;
    ovf=(high+fq/2)/fq;
}
/**
 * 
 * @param pin
 * @param scaler
 * @param overFlow
 */
void pwmFromScalerAndOverflow(int pin, int scaler, int overFlow)
{
    
    timer_dev *tdev=PIN_MAP[pin].timer_device;
    HardwareTimer *t=pinToTimer(tdev);

    int channel=PIN_MAP[pin].timer_channel;
    if(!channel) xAssert(0);
    
    t->pause();  
    t->setPrescaleFactor(scaler);
    t->setOverflow(overFlow);
    t->setCompare(channel,overFlow/2);
    t->setCount(0);
    tdev->regs.bas->CR1|=0x10; //downcounting
    t->refresh();
    gpio_set_mode(PIN_MAP[pin].gpio_device, PIN_MAP[pin].gpio_bit, GPIO_AF_OUTPUT_PP);

    timer_disable_irq(tdev, channel);
    timer_oc_set_mode(tdev, channel, TIMER_OC_MODE_PWM_1, TIMER_OC_PE);
    timer_cc_enable(tdev, channel);
}
/**
 * 
 * @param t
 * @param scaler
 * @param overFlow
 */
void pwmFromScalerAndOverflow(HardwareTimer *t,int channel, int scaler, int overFlow)
{
    
    
    t->pause();  
    t->setPrescaleFactor(scaler);
    t->setOverflow(overFlow);
    t->setCompare(channel,overFlow/2);
    t->setCount(0);
    t->c_dev()->regs.bas->CR1|=0x10; //downcounting
    t->refresh();

    timer_disable_irq(t->c_dev(), channel);
    timer_oc_set_mode(t->c_dev(), channel, TIMER_OC_MODE_PWM_1, TIMER_OC_PE);
    timer_cc_enable(t->c_dev(), channel);
}

/**
 * 
 * @param pin
 * @param fq
 */
void myPwm(int pin,int fq)
{
    int scaler;
    int overFlow;
    
    pwmGetScaleOverFlow(fq,scaler,overFlow);
    pwmFromScalerAndOverflow(pin,scaler,overFlow);
}
void pwmPause(int pin)
{
    timer_dev *tdev=PIN_MAP[pin].timer_device;
    HardwareTimer *t=pinToTimer(tdev);
    t->setCount(0);
    t->refresh();    
    t->pause();
}
void pwmRestart(int pin)
{
    timer_dev *tdev=PIN_MAP[pin].timer_device;
    HardwareTimer *t=pinToTimer(tdev);
    
    t->setCount(0);
    t->refresh();
    t->resume();
}


