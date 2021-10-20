
#include "lnArduino.h"
#include "dso_adc_capture.h"


/*
 * Partially filled global gain array
 * Remaining columns will be filled at runtime
 */
#define SAT_THRESHOLD 66 //5%
#define SWING(x) (2048-x+SAT_THRESHOLD)


VoltageSettings vSettings[NB_CAPTURE_VOLTAGE]= {
    {"GND",     DSOInputGain::MAX_VOLTAGE_GND,  24000.,    SWING(2048) },
    {"5mv",     DSOInputGain::MAX_VOLTAGE_20MV,  4800.,    SWING(2048)}, // "Digital" zoom
    {"10mv",    DSOInputGain::MAX_VOLTAGE_20MV,  2400.,    SWING(2048)},
    {"20mv",    DSOInputGain::MAX_VOLTAGE_40MV , 1200.,    SWING(2048)},
    {"50mv",    DSOInputGain::MAX_VOLTAGE_80MV,  480.,     SWING(2048)},
    {"100mv",   DSOInputGain::MAX_VOLTAGE_200MV,  240.,    SWING(2048)},
    {"200mv",   DSOInputGain::MAX_VOLTAGE_250MV,  120.,    SWING(1100)}, // that one will saturate early
    //{"500mv",   DSOInputGain::MAX_VOLTAGE_800MV,  48.},
    {"500mV",   DSOInputGain::MAX_VOLTAGE_2V,  48.,        SWING(2048)}, // "digital" zoom
    {"1v",      DSOInputGain::MAX_VOLTAGE_2V,  24.,        SWING(2048)},
    {"2v",      DSOInputGain::MAX_VOLTAGE_4V, 12.,         SWING(2048)},
    {"5v",      DSOInputGain::MAX_VOLTAGE_8V, 4.8,         SWING(2048)}
};

const TimerTimeBase timerBases[]=
{
    { DSOCapture::DSO_TIME_BASE_25US,   "25us", 960*1000    , 1, LN_ADC_SMPT_1_5,    lnADC_CLOCK_DIV_BY_4 }, // 1 us / sample round up, error =0.3%%
    { DSOCapture::DSO_TIME_BASE_50US,   "50us", 480*1000    , 1, LN_ADC_SMPT_1_5,    lnADC_CLOCK_DIV_BY_8 }, // 2 us / sample => 0.5 with OS, no oversampling
    { DSOCapture::DSO_TIME_BASE_100US,  "100us",240*1000    , 1, LN_ADC_SMPT_41_5,   lnADC_CLOCK_DIV_BY_4 },    // 100/24= 4 us , 1 us with os
    { DSOCapture::DSO_TIME_BASE_200US,  "200us",120*1000    , 1, LN_ADC_SMPT_239_5,  lnADC_CLOCK_DIV_BY_2 },    // 100/24= 4 us , 1 us with os    
    { DSOCapture::DSO_TIME_BASE_500US,  "500us",48*1000     , 1, LN_ADC_SMPT_239_5,  lnADC_CLOCK_DIV_BY_4 },    // 500/24=> 20 us /sample => 5 us with over
    { DSOCapture::DSO_TIME_BASE_1MS,    "1ms",  24*1000     , 1, LN_ADC_SMPT_239_5,  lnADC_CLOCK_DIV_BY_8 }, // 40 us / sample => 10 us with oversampling
    { DSOCapture::DSO_TIME_BASE_2MS,    "2ms",  12*1000     , 1, LN_ADC_SMPT_239_5,  lnADC_CLOCK_DIV_BY_8 }, // 40 us / sample => 10 us with oversampling    
    { DSOCapture::DSO_TIME_BASE_5MS,    "5ms",  4800        , 1, LN_ADC_SMPT_239_5,  lnADC_CLOCK_DIV_BY_8 }, // 200 us / sample => 50 us with os
    { DSOCapture::DSO_TIME_BASE_10MS,   "10ms", 2400        , 1, LN_ADC_SMPT_239_5,  lnADC_CLOCK_DIV_BY_8 }, // 400 us / 100 us with os    **20
    { DSOCapture::DSO_TIME_BASE_20MS,   "20ms", 1200        , 1, LN_ADC_SMPT_239_5,  lnADC_CLOCK_DIV_BY_8 }, // 400 us / 100 us with os    **20    
    { DSOCapture::DSO_TIME_BASE_50MS,   "50ms", 480         , 1, LN_ADC_SMPT_239_5,  lnADC_CLOCK_DIV_BY_8 },  // 2 ms      **71
#if 0            
    { DSOCapture::DSO_TIME_BASE_100MS,  "100ms",240         , 1, ADC_SMPR_239_5,  DSOADC::ADC_PRESCALER_8 }, // 4 ms      **142
    { DSOCapture::DSO_TIME_BASE_200MS,  "200ms",120         , 1, ADC_SMPR_239_5,  DSOADC::ADC_PRESCALER_8 }, // 4 ms      **142    
    { DSOCapture::DSO_TIME_BASE_500MS,  "500ms",48          , 1, ADC_SMPR_239_5,  DSOADC::ADC_PRESCALER_8 },  // 20 ms     **714
    { DSOCapture::DSO_TIME_BASE_1S,     "1s",   24          , 1, ADC_SMPR_239_5,  DSOADC::ADC_PRESCALER_8 }   // 40 ms     **1428
#endif
 };
