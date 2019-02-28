/***************************************************
 STM32 duino based firmware for DSO SHELL/150
 *  * GPL v2
 * (c) mean 2019 fixounet@free.fr
 ****************************************************/
#pragma once
#include <Wire.h>


// Sampling Queue

#define SAMPLING_QUEUE_SIZE 3
#define maxSamples   (360) //1024*6
/* 
 * \brief Describe a voltage setting
 */

typedef struct VoltageSettings
{
    const char *name;           /// name of the setting i.e 10 ms/div
    int         inputGain;      /// Input gain selected 4051/4053 switch 
    float       displayGain;    /// multiply by this to get pixels from volt
    float       multiplier;     /// Gain of the internal amplifier, multiply by this to get volts
    int         offset;         /// Offset of sampled data , you need to substract it

};

typedef struct TimeSettings
{
  const char    *name;
  adc_prescaler  prescaler;
  adc_smp_rate   rate;
  int            expand4096;
};

/**
 * \class SampleingQueue
 * \brief simple std::vector alternative to store free buffers & buffers containing captured data
 */
class SampleingQueue
{
public:
    SampleingQueue()
    {
        count=0;
    }
    void addFromIsr(uint32_t *ptr)
    {
        data[count++]=ptr;
    }
    void add(uint32_t *ptr)
    {
        noInterrupts();
        addFromIsr(ptr);
        interrupts();
    }
    bool empty()
    {
        return !count;
    }
    uint32_t *takeFromIsr()
    {
        if(!count) return NULL;
        uint32_t *out=data[0];
        memmove(data,data+1,(count-1)*4);
        count--;
        return out;
    }
    uint32_t *take()
    {
        uint32_t *out;
        noInterrupts();
        out=takeFromIsr();
        interrupts();
        return out;
    }

protected:
    uint32_t *data[SAMPLING_QUEUE_SIZE];
    int      count;
    
    
};

/**
 * \class DSOADC
 * \brief wrapper on top of the DMA-ADC engine
 */
class DSOADC
{
public:
                    DSOADC();
            bool    setTimeScale(adc_smp_rate one, adc_prescaler two);
            bool    prepareDMASampling ();
            bool    prepareTimerSampling (int fq);
            uint32_t *getSamples(int &count);
            void     reclaimSamples(uint32_t *buffer);
            bool     setSlowMode(int fqInHz);
    static  uint32_t getVCCmv();
            
            

            bool startDMASampling (int count);
            bool startTimerSampling (int count);
protected:            
            void setADCs ();
    static  void adc_dma_disable(const adc_dev * dev) ;            
    static  void adc_dma_enable(const adc_dev * dev) ;    
    static  void DMA1_CH1_Event();
            void captureComplete();
    static  void Timer2_Event();
            void timerCapture();
            bool startInternalDmaSampling ();
            
            

protected:
  
            int             _sampled;
            SampleingQueue availableBuffers;
            SampleingQueue capturedBuffers;
            
};

// EOF
