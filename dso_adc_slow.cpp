/**
 * Derived from https://github.com/pingumacpenguin/STM32-O-Scope/blob/master/STM32-O-Scope.ino
 */
/*.
(c) Andrew Hull - 2015
STM32-O-Scope - aka "The Pig Scope" or pigScope released under the GNU GENERAL PUBLIC LICENSE Version 2, June 1991
https://github.com/pingumacpenguin/STM32-O-Scope
Adafruit Libraries released under their specific licenses Copyright (c) 2013 Adafruit Industries.  All rights reserved.
*/
/**
 We use PA0 as input pin
 * DMA1, channel 0
 
 */

#include "dso_global.h"

/**
 */

#define analogInPin  PA0


extern HardwareTimer Timer2;
extern adc_reg_map *adc_Register;

/**
 */
extern int requestedSamples;
extern uint32_t *currentSamplingBuffer;
extern DSOADC             *instance;
int currentIndex=0;

/**
 * 
 * @param fqInHz
 * @return 
 */
bool DSOADC::setSlowMode(int fqInHz)
{
    
    Timer2.setChannel1Mode(TIMER_OUTPUTCOMPARE);
    Timer2.setPeriod(1000000/fqInHz); // in microseconds
    Timer2.setCompare1(1); // overflow might be small
    Timer2.attachCompare1Interrupt(Timer2_Event);

}

  /**
  * 
  * @param count
  * @return 
  */
bool    DSOADC::initiateTimerSampling (int count)
{    
    if(!capturedBuffers.empty())
        return true; // We have data !
    
    uint32_t *bfer=availableBuffers.take();
    if(!bfer) return false;
    
    return startTimerSampling(count,bfer);
    
}
/**
 * 
 * @param count
 * @param buffer
 * @return 
 */
bool DSOADC::startTimerSampling (int count,uint32_t *buffer)
{
    if(count>maxSamples)
        count=maxSamples;   
    currentIndex=0;
    requestedSamples=count;
    currentSamplingBuffer=buffer;
    currentIndex=0;
    adc_dev *dev = PIN_MAP[PA0].adc_device;
    uint32 tmp = dev->regs->SQR1;
    tmp &= ~ADC_SQR1_L;
    adc_Register->SQR3 = 0;
    setSlowMode(4800);
    adc_Register->CR2 |= ADC_CR2_SWSTART;    
    adc_Register->SQR1 = tmp;
    // start timer
} 
void DSOADC::Timer2_Event() 
{
    instance->timerCapture();
}
/**
 * \fn timerCapture
 * \brief this is one is called by a timer interrupt
 */
void DSOADC::timerCapture()
{
    // read previous cap
    currentSamplingBuffer[currentIndex++]=adc_Register->DR;
    // start new one
    
    if(currentIndex>=requestedSamples)
    {
        captureComplete();
        return;
    }
    adc_Register->CR2 |= ADC_CR2_SWSTART;        
}


