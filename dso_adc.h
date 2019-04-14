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

typedef struct SampleSet
{
  int       samples;
  uint32_t  *data;
};

static bool triggered=false;

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
    void addFromIsr(SampleSet *ptr)
    {
        dataSet[count++]=ptr;
    }
    void add(SampleSet *ptr)
    {
        noInterrupts();
        addFromIsr(ptr);
        interrupts();
    }
    bool empty()
    {
        return !count;
    }
    SampleSet *takeFromIsr()
    {
        if(!count) return NULL;
        SampleSet *out=dataSet[0];
        memmove(dataSet,dataSet+1,(count-1)*4);
        count--;
        return out;
    }
    SampleSet *take()
    {
        SampleSet *out;
        noInterrupts();
        out=takeFromIsr();
        interrupts();
        return out;
    }

protected:
    int       count;
    SampleSet *dataSet[SAMPLING_QUEUE_SIZE];
    
    
};




/**
 * \class DSOADC
 * \brief wrapper on top of the DMA-ADC engine
 */
class DSOADC
{
public:  
  enum TriggerMode
  {
    Trigger_Rising,
    Trigger_Falling,
    Trigger_Both,
  };
  
  
public:
                    DSOADC();
            bool    setTimeScale(adc_smp_rate one, adc_prescaler two);
            bool    prepareDMASampling (adc_smp_rate rate,adc_prescaler scale);
            bool    prepareTimerSampling (int fq);
            SampleSet *getSamples();
            void     reclaimSamples(SampleSet *buffer);
            bool     setSlowMode(int fqInHz);
            bool     readCalibrationValue();
    static  uint32_t getVCCmv();
            bool     setTriggerMode(TriggerMode mode);
            bool     getTriggerState();
            bool     setVrefPWM(int ratio); // Trigger 
            

            bool startDMASampling (int count);
            bool startTimerSampling (int count);
            bool startTriggeredTimerSampling (int count,float triggerValue);
            void clearCapturedData();
protected:            
            void setADCs ();
    static  void adc_dma_disable(const adc_dev * dev) ;            
    static  void adc_dma_enable(const adc_dev * dev) ;    
    static  void DMA1_CH1_Event();
            void captureComplete();
    static  void Timer2_Event();
    static  void Timer2Trigger_Event();

            void timerCapture();
            void timerTriggerCapture();
            
            bool startInternalDmaSampling ();
            
            
    static void TriggerInterrupt();
            
            

protected:
  
            int             _sampled;
            SampleingQueue availableBuffers;
            SampleingQueue capturedBuffers;
            
};

// EOF

