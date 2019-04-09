/***************************************************
 STM32 duino based firmware for DSO SHELL/150
 *  * GPL v2
 * (c) mean 2019 fixounet@free.fr
 ****************************************************/
#pragma once
/**
 */
#include "transform.h"
/**
 */
class DSOCapture
{
public:
    enum DSO_CAPTURE_STATE
    {
      DSO_STATE_RUN=0,
      DSO_STATE_TRIGGERED=1
    };
    enum 
    {
      DSO_MODE_CONTINOUS,
      DSO_MODE_TRIGGERED_SINGLE,
      DSO_MODE_TRIGGERED_CONTINUOUS,
    };
    enum DSO_TIME_BASE 
    {
      DSO_TIME_BASE_10US,
      DSO_TIME_BASE_25US,
      DSO_TIME_BASE_50US,
      DSO_TIME_BASE_100US,
      DSO_TIME_BASE_500US,
      DSO_TIME_BASE_1MS, // Fast
      DSO_TIME_BASE_5MS, // SLow
      DSO_TIME_BASE_10MS,
      DSO_TIME_BASE_50MS,
      DSO_TIME_BASE_100MS,
      DSO_TIME_BASE_500MS,
      DSO_TIME_BASE_1S,
      DSO_TIME_BASE_MAX=DSO_TIME_BASE_1S
    };
    
    enum DSO_VOLTAGE_RANGE
    {
      DSO_VOLTAGE_1MV,
      DSO_VOLTAGE_5MV,
      DSO_VOLTAGE_10MV,
      DSO_VOLTAGE_20MV,
      DSO_VOLTAGE_50MV,
      DSO_VOLTAGE_100MV,
      DSO_VOLTAGE_200MV,
      DSO_VOLTAGE_500MV,
      DSO_VOLTAGE_1V,
      DSO_VOLTAGE_2V,
      DSO_VOLTAGE_5V,
      DSO_VOLTAGE_MAX=DSO_VOLTAGE_5V
    };
    static int         oneShotCapture(int count,float *outbuffer,CaptureStats &stats) ;
    static int         triggeredCapture(int count,float *outbuffer,CaptureStats &stats);
    static bool        setVoltageRange(DSO_VOLTAGE_RANGE voltRange);
    static DSO_VOLTAGE_RANGE getVoltageRange();
    static bool        setTimeBase(DSO_TIME_BASE timeBase);
    static DSO_TIME_BASE getTimeBase();
    static bool        prepareSampling ();
    static bool        startSampling (int count);
    static SampleSet   *getSamples();
    static void        reclaimSamples(SampleSet *set);
    
    static bool        captureToDisplay(int count,float *samples,uint8_t *waveForm);
    static const char *getTimeBaseAsText();
    static const char *getVoltageRangeAsText();
    static void        clearCapturedData();
    
protected:
  
};
