/***************************************************
 STM32 duino based firmware for DSO SHELL/150
 *  * GPL v2
 * (c) mean 2019 fixounet@free.fr
 ****************************************************/
#pragma once
#include "Arduino.h"
#include "dma.h"
#include "dso_adc_gain.h"

// Sampling Queue

#define ADC_INTERNAL_BUFFER_SIZE (1024)
/* 
 * \brief Describe a voltage setting
 */

typedef struct VoltageSettings
{
    const char          *name;          /// name of the setting i.e 10 ms/div
    DSOInputGain::InputGainRange gain;
    float               displayGain;    /// multiply by this to get pixels from volt
    int                 maxSwing;
};

/**
 * 
 */
class SampleSet
{
public:
            SampleSet()
            {
              samples=0;
              data=NULL;
            }
            void set(int s, uint16_t *d)
            {
              samples=s;
              data=d;
            }
            
            SampleSet(int s, uint16_t *d)
            {
              set(s,d);
            }
  int       samples;
  uint16_t  *data;
};

/**
 */
typedef struct FullSampleSet
{
  SampleSet set1;
  SampleSet set2;  
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
    Trigger_Run
  };
   enum TriggerState
  {
    Trigger_Preparing,
    Trigger_Armed
  };
  enum Prescaler
  {
    ADC_PRESCALER_2=0,
    ADC_PRESCALER_4=1,
    ADC_PRESCALER_6=2,
    ADC_PRESCALER_8=3,
    
#ifdef HIGH_SPEED_ADC
    ADC_PRESCALER_12= 5,
    ADC_PRESCALER_16= 7,
    ADC_PRESCALER_5=  8,
    ADC_PRESCALER_10= 10,
    ADC_PRESCALER_20= 11,
#endif    
    

  };
public:
                    DSOADC(int pin);
            bool    setADCPin(int pin);
            void    setChannel(int channel);
            bool    setTimeScale(adc_smp_rate one, DSOADC::Prescaler two);
            bool    prepareDMASampling (adc_smp_rate rate,DSOADC::Prescaler scale);
            bool    prepareDualDMASampling (int otherPin, adc_smp_rate rate,DSOADC::Prescaler  scale);
            bool    startDualDMASampling (const int otherPin, const int count);
            bool    prepareTimerSampling (int fq);
            int     pollingRead();
            bool    startDMA();
            bool    startDualDMA();
            bool    getSamples(FullSampleSet &fullSet)           ;
            void    clearSemaphore() ;
            bool     setSlowMode(int fqInHz);
            bool     readCalibrationValue();
    static  uint32_t getVCCmv(); // this one is cached
    static  float    readVCCmv();    
    static float     adcToVolt(float adc);
            bool     setTriggerMode(TriggerMode mode);
            TriggerMode getTriggerMode() {return _triggerMode;};
            bool     getTriggerState();
            bool     setVrefPWM(int ratio); // Trigger 
            void     setupAdcDualDmaTransfer( int otherPin,  int count,uint32_t *buffer, void (*handler)(void) );

            bool startDMASampling (const int count);
            bool startDMATriggeredSampling (const int count, int ADCTriggerValue);
            bool startTimerSampling (int count);
            bool startTriggeredTimerSampling (int count,uint32_t triggerADC);
            void clearSamples();
    static  void adc_dma_disable(const adc_dev * dev) ;            
    static  void adc_dma_enable(const adc_dev * dev) ;    

            void setupADCs ();
protected:            
            
    static  void DMA1_CH1_Event();
    static  void DMA1_CH1_TriggerEvent() ;
            void captureComplete(SampleSet &one, SampleSet &two);
    static  void Timer_Event();
    static  void Timer_Trigger_Event();

            void timerCapture();
            void timerTriggerCapture();
            
            bool startInternalDmaSampling ();
            
            bool validateAverageSample(uint32_t &avg);
    static void TriggerInterrupt();
public:        
    static void setupAdcDmaTransfer(   int count,uint16_t *buffer, void (*handler)(void) );
    static void nextAdcDmaTransfer( int count,uint16_t *buffer);

    static void enableDisableIrqSource(bool onoff, int interruptMask);
    static void enableDisableIrq(bool onoff);
    static void defaultAdcIrqHandler();
    static void attachWatchdogInterrupt(voidFuncPtr handler);
    static void setWatchdogTriggerValue(uint32_t high, uint32_t low);
    static void watchDogInterrupt();
            void awdTrigger() ;                       
            bool awdTriggered() {return _triggered;}
            void stopDmaCapture();
            void stopTimeCapture();
    static  void getRegisters();
            void resetStats();
    
            //void restartDmaTriggerCapture() ;
protected:
            int             _pin;
            int             _sampled;
            FullSampleSet   _captured;
            TriggerMode     _triggerMode;
            bool            _triggered;
            TriggerState    _triggerState;
            int             _triggerValueADC;
            static bool     _dual;
public:            
static      uint16_t adcInternalBuffer[ADC_INTERNAL_BUFFER_SIZE];            
};


/**
 */
typedef struct TimeSettings
{
  bool               dual;
  const char         *name;
  DSOADC::Prescaler  prescaler;
  adc_smp_rate       rate;
  int                expand4096;
  int                fqInHz;
};

// EOF

