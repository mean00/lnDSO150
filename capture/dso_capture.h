
#pragma once
#include "lnADC.h"
#include "dso_adc_gain.h"
#include "lnGPIO.h"
#define NB_CAPTURE_VOLTAGE (11)     
#define DSO_NB_TIMEBASE (11)
#define SLOWER_FAST_MODE     DSO_TIME_BASE_10US

typedef void (captureCb)();

/**
 */
struct VoltageSettings
{
    const char          *name;          /// name of the setting i.e 10 ms/div
    DSOInputGain::InputGainRange gain;
    float               displayGain;    /// multiply by this to get pixels from volt
    int                 maxSwing;
};
//---------------

extern VoltageSettings vSettings[NB_CAPTURE_VOLTAGE];
class lnDSOAdc;
/**
 */
class DSOCapture
{
public:
   enum TriggerMode
    {
        Trigger_Rising=0,
        Trigger_Falling=1,
        Trigger_Both=2,
        Trigger_Run=3
    };       
    enum DSO_TIME_BASE 
    {
      //DSO_TIME_BASE_5US=0,DSO_TIME_MIN=0,
      DSO_TIME_BASE_10US=0,DSO_TIME_MIN=0,
      DSO_TIME_BASE_20US,
      DSO_TIME_BASE_50US,
      DSO_TIME_BASE_100US,
      DSO_TIME_BASE_200US,
      DSO_TIME_BASE_500US, // FAST
      DSO_TIME_BASE_1MS,  // SLow
      DSO_TIME_BASE_2MS, 
      DSO_TIME_BASE_5MS, 
      DSO_TIME_BASE_10MS,
      DSO_TIME_BASE_20MS,
      DSO_TIME_BASE_50MS,
      DSO_TIME_BASE_100MS,
      DSO_TIME_BASE_200MS,
      DSO_TIME_BASE_500MS,
      DSO_TIME_BASE_1S,
      DSO_TIME_BASE_MAX=DSO_TIME_BASE_1S
    };
    enum DSO_VOLTAGE_RANGE
    {
      DSO_VOLTAGE_GND,  // 0
      
      DSO_VOLTAGE_5MV,  // 1
      DSO_VOLTAGE_10MV, // 2
      DSO_VOLTAGE_20MV, // 3
      DSO_VOLTAGE_50MV, // 4
      
      DSO_VOLTAGE_100MV,    // 5  
      DSO_VOLTAGE_200MV,// 6
      DSO_VOLTAGE_500MV,// 7
      DSO_VOLTAGE_1V,   // 8
      DSO_VOLTAGE_2V,   // 9
      DSO_VOLTAGE_5V,   // 10
      
      DSO_VOLTAGE_MAX=DSO_VOLTAGE_5V
    };
  
    
public:
    static void                            initialize(lnPin pin);
    static void                            setTriggerMode(TriggerMode mode);
    static TriggerMode                     getTriggerMode();
    static const char *                    getTriggerModeAsText();
    static float                           getTriggerVoltage();
    static void                            setTriggerVoltage(const float &s);
    static bool                            setVoltageRange(DSOCapture::DSO_VOLTAGE_RANGE voltRange);
    static DSOCapture::DSO_VOLTAGE_RANGE   getVoltageRange();
    static const char *                    getVoltageRangeAsText();
    //
    static void                            setTimeBase(DSO_TIME_BASE timeBase);
    static DSO_TIME_BASE                   getTimeBase();
    static const char *                    getTimeBaseAsText();

    static                                  void setCb(captureCb *cb);
    static                                  bool getData(int &nb, float *f);
    static                                  bool startCapture(int nb);
    static                                  void stopCapture();

    static void                             captureDone(int nb);
    static float                            getVoltToPix();
    static int                              computeFrequency();
protected:
    static int                             computeFrequency_(int xsamples,uint16_t *data);
    static int                             currentVoltageRange;
    static DSO_TIME_BASE                   currentTimeBase; 
    

    static  lnPin                           _pin;
    static  captureCb                       *_cb;
    static  int                             _nb;
    
public:    
    enum captureState
    {
        CAPTURE_STOPPED=0,        
        CAPTURE_RUNNING=1,
        CAPTURE_DONE=2, // It is done but not cleaned up yet
    };
    static captureState                     _state;
    static lnDSOAdc                         *_adc;
    static float                            _triggerVolt;
    static int                              _triggerAdc;
    static TriggerMode                      _triggerMode;
};



/**
 */
struct TimerTimeBase
{
  DSOCapture::DSO_TIME_BASE timeBase;
  const char    *name;
  int           fq;  
};

struct TimerTimeADC
{
  DSOCapture::DSO_TIME_BASE timeBase;
  int           overSampling; // Oversampling ratio, only on GD32!
  lnADC_CYCLES  rate ;
  lnADC_DIVIDER scale;  
};
#define DSO_NB_TIMESCALE 12
extern const TimerTimeBase timerBases[];
extern  TimerTimeADC  timerADC[];
// EOF