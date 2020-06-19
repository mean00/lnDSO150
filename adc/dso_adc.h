/***************************************************
 STM32 duino based firmware for DSO SHELL/150
 *  * GPL v2
 * (c) mean 2019 fixounet@free.fr
 ****************************************************/
#pragma once
#include "Arduino.h"
#include "dma.h"


// Sampling Queue

#define ADC_INTERNAL_BUFFER_SIZE (1024)

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

  enum ADC_CAPTURE_MODE
  {
      ADC_CAPTURE_MODE_NORMAL=0,
      ADC_CAPTURE_FAST_INTERLEAVED=1,
      ADC_CAPTURE_SLOW_INTERLEAVED=2
  };
  enum ADC_TRIGGER_SOURCE
  {
    ADC_SOURCE_TIMER_1_CHANNEL1 =0,
    ADC_SOURCE_TIMER_1_CHANNEL2 =1,
    ADC_SOURCE_TIMER_1_CHANNEL3 =2,
    ADC_SOURCE_TIMER_2_CHANNEL2 =3,
    ADC_SOURCE_TIMER_3_TRGO     =4,
    ADC_SOURCE_TIMER_4_CHANNEL4 =5,
    ADC_SOURCE_SWSTART          =7,
  };  
  enum Prescaler
  {
    ADC_PRESCALER_2=0,
    ADC_PRESCALER_4=1,
    ADC_PRESCALER_6=2,
    ADC_PRESCALER_8=3,   
    // The ones below only works with GD32 chip
    ADC_PRESCALER_12= 5,
    ADC_PRESCALER_16= 7,
    ADC_PRESCALER_5=  8,
    ADC_PRESCALER_10= 10,
    ADC_PRESCALER_20= 11,
    

  };
public:

            bool    setOverSamplingFactor  (int overSamp); // Does nothing on stm32
                    DSOADC(int pin);
            bool    setSource(const ADC_TRIGGER_SOURCE source);            
            bool    setADCPin(int pin);
            void    setChannel(int channel);
            bool    setTimeScale(adc_smp_rate one, DSOADC::Prescaler two);
            bool    prepareDMASampling (adc_smp_rate rate,DSOADC::Prescaler scale);
            bool    prepareFastDualDMASampling (int otherPin, adc_smp_rate rate,DSOADC::Prescaler  scale);
            bool    prepareSlowDualDMASampling (int otherPin, adc_smp_rate rate,DSOADC::Prescaler  scale);
            bool    startDualDMASampling (const int otherPin, const int count);
            bool    prepareTimerSampling (int fq,bool overSampling,adc_smp_rate rate,DSOADC::Prescaler  scale );
            int     pollingRead();
            bool    startDMA();
            bool    startDMATime();
            bool    startDualDMA();
            bool    getSamples(FullSampleSet &fullSet)           ;
            bool    getSamples(uint16_t **samples, int  &nbSamples);
            void    clearSemaphore() ;
            bool     setSlowMode(int fqInHz);
            bool     readCalibrationValue();
    static  uint32_t getVCCmv(); // this one is cached
    static  float    readVCCmv();    
    static float     adcToVolt(float adc);
            bool     setTriggerMode(TriggerMode mode);
            TriggerMode getTriggerMode() {return _triggerMode;};
            TriggerMode getActualTriggerMode() 
                        {
                            if(_triggerMode!=Trigger_Both) return _triggerMode;
                            return _both;
                        };
            bool     getTriggerState();
            bool     setVrefPWM(int ratio); // Trigger 
            void     setupAdcDualDmaTransfer( int otherPin,  int count,uint32_t *buffer, void (*handler)(void) ,bool circular);

            bool startDMASampling (const int count);
            bool startDMATriggeredSampling (const int count, int ADCTriggerValue);
            bool commonTrigger (int count,uint32_t triggerValueADC);
            bool startTimerSampling (int count);
            bool startTriggeredTimerSampling (int count,uint32_t triggerADC);
    static  void clearSamples();
    static  void adc_dma_disable(const adc_dev * dev) ;            
    static  void adc_dma_enable(const adc_dev * dev) ;    

            void setupADCs ();
            bool fastSampleDown(int threshold,int &value, int &timeUs)  ;
            bool fastSampleUp(int threshold1,int threshold2,int &value1,int &value2, int &timeUs1,int &timeUs2)  ;
            
protected:            
            bool setSourceInternal();
    static  void DMA1_CH1_Event();
    static  void DMA1_CH1_TriggerEvent() ;
            void captureComplete(SampleSet &one, SampleSet &two);

            
            bool startInternalDmaSampling ();
            
            bool validateAverageSample(uint32_t &avg);    
public:        
    static void setupAdcDmaTransfer(   int count,uint16_t *buffer, void (*handler)(void),bool circular );
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
            bool setupTimerSampling(); // this is to be called once when switching from DMA to timer
            bool setupDmaSampling();   // same when switching back
protected:
            int             _pin;
            int             _sampled;
            FullSampleSet   _captured;
            TriggerMode     _triggerMode,_both;
            bool            _triggered;
            TriggerState    _triggerState;
            int             _triggerValueADC;
            int             _oldTimerFq; // old timer frequency value
  static    ADC_CAPTURE_MODE     _dual;
            ADC_TRIGGER_SOURCE _source; // source of sampling signal : SWSTART or Timer
            bool               _overSampling;
            adc_smp_rate       _timerSamplingRate;
            DSOADC::Prescaler  _timerScale;
public:            
static      uint16_t adcInternalBuffer[ADC_INTERNAL_BUFFER_SIZE];            
};


// EOF

