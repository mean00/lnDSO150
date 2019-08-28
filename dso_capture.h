/***************************************************
 STM32 duino based firmware for DSO SHELL/150
 *  * GPL v2
 * (c) mean 2019 fixounet@free.fr
 ****************************************************/
#pragma once
/**
 */
typedef struct
{
  float xmin;
  float xmax;
  float avg;
  int   trigger;   // -1 = no trigger; else offset
  int   frequency; //  0 or -1= unknown
}CaptureStats;

/**
 */
typedef struct CapturedSet
{
    int          samples;
    float        data[240];    
    CaptureStats stats;
};
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
#define NB_ADC_VOLTAGE 13
#define NB_CAPTURE_VOLTAGE (11)     
    
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
    // capture
    static int         capture(int count,float *voltage,CaptureStats &stats);    
    static void        stopCapture();        
    
    // Voltage Range    
    static bool        setVoltageRange(DSO_VOLTAGE_RANGE voltRange);
    static DSO_VOLTAGE_RANGE getVoltageRange();
    static const char *getVoltageRangeAsText();
    static float       getVoltageRangeAsFloat(DSO_VOLTAGE_RANGE range);
    static int         getVoltageRangeIndex(DSO_VOLTAGE_RANGE range);
    // Time Range
    static bool        setTimeBase(DSO_TIME_BASE timeBase);
    static DSO_TIME_BASE getTimeBase();
    static const char *getTimeBaseAsText();
    static int         timeBaseToFrequency(DSO_TIME_BASE timeBase);
    // Trigger
    static void        updateTriggersValue();
    static void        setTriggerValue(float volt);
    static float       getTriggerValue();
    static void        setTriggerMode(TriggerMode mode);
    static TriggerMode getTriggerMode();
    
   
    
    // Voltage Offset
    static void        setVoltageOffset(float volt);
    static float       getVoltageOffset();

    // Misc
    static bool        captureToDisplay(int count,float *samples,uint8_t *waveForm);
    static void        clearCapturedData();
    static int         voltageToPixel(float v);
    static void        initialize();    
    
    static int          voltToADCValue(float v);
    
    // needed by autoSetup
    static float        getMaxVoltageValue();
    static float        getMinVoltageValue();

};

