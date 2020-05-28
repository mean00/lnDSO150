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
 
 * Vref is using PWM mode for Timer4/Channel 3
 * 
 * Correct init order is 
 *     ADC
 *     DMA
 *     SWSTART
 * 
 */
#include "dso_adc.h"
#include "fancyLock.h"
#include "dma.h"
#include "adc.h"
extern int  readAllRegisters();
extern uint32_t cr2;
/**
 */
bool    DSOADC::fastSampleDown(int threshold,int &value, int &timeUs)  
{
    setTimeScale(ADC_SMPR_13_5, ADC_PRESCALER_6); // about 2 us sampling fq
    adc_dev *dev = PIN_MAP[_pin].adc_device;
    adc_reg_map *regs = dev->regs;           
    setChannel(PIN_MAP[_pin].adc_channel);    
    regs->CR2 &= ~(ADC_CR2_DMA|ADC_CR2_CONT);    
    uint32_t val=regs->DR ; // clear pending value
    
    // go
    int c;
    uint32_t start=micros();
    uint32_t sampleTime;
    bool first=true;
    value=regs->DR ; // clear pending value
    
    
    while(1)
    {
        regs->CR2|=ADC_CR2_SWSTART;
        cr2=regs->CR2;
        uint32_t sampleStart=millis();
        while(1)
        {
            uint32_t sr=regs->SR;
            if(!(sr & ADC_SR_EOC))
            {
                int now=millis();
                if((now-sampleStart)>10)
                {
                    return false;
                }
            }
            sampleTime=micros();
            break;
        }
        value=regs->DR & ADC_DR_DATA;
        if(value<threshold)
        {
            timeUs=sampleTime-start; 
            return true;
        }
    }    
}


/**
 * 
 * @param threshold
 * @param value
 * @return 
 */
bool    DSOADC::fastSampleUp(int threshold1,int threshold2,int &value1,int &value2, int &timeUs1,int &timeUs2)
{
    
    setTimeScale(ADC_SMPR_13_5, ADC_PRESCALER_6); // about 2 us sampling fq
    adc_dev *dev = PIN_MAP[_pin].adc_device;
    adc_reg_map *regs = dev->regs;           
    setChannel(PIN_MAP[_pin].adc_channel);    
    regs->CR2 &= ~(ADC_CR2_DMA|ADC_CR2_CONT);    
    uint32_t val=regs->DR ; // clear pending value
      // go
    int c;
    uint32_t timeout=millis();
    uint32_t start=micros();
    uint32_t sampleTime;
    bool first=true;
    uint32_t value=regs->DR ; // clear pending value
    value1=0;
    value2=0;
    timeUs1=0;
    timeUs2=0;
    while(1)
    {
       // if((millis()-timeout)>10000) return false; // timeout!
        regs->CR2|=ADC_CR2_SWSTART;
        uint32_t sampleStart=millis();
        while(1)
        {
            uint32_t sr=regs->SR;
            if(!(sr & ADC_SR_EOC))
            {
                int now=millis();
                if((now-sampleStart)>10)
                {
                    return false;
                }
            }
            sampleTime=micros();
            break;
        }
        value=regs->DR & ADC_DR_DATA;
        if(first)
        {
            if(value>threshold1)
            {
                timeUs1=sampleTime-start; 
                value1=value;
                first=false;
            }
        }
        else
        {
             if(value>threshold2)
            {
                timeUs2=sampleTime-start; 
                value2=value;
                return true;
            }
        }
     
    }
}