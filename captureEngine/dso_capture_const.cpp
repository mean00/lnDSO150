/***************************************************
 STM32 duino based firmware for DSO SHELL/150
 *  * GPL v2
 * (c) mean 2019 fixounet@free.fr
 ****************************************************/

#include "dso_global.h"
#include "dso_adc.h"
#include "dso_capture.h"
#include "dso_capture_priv.h"

#include "DSO_config.h"
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


/**
 These the time/div settings, it is computed to maximum accuracy 
 * and sample a bit too fast, so that we can decimate it
 *  */
const TimeSettings tSettings[]
{
#ifdef       HIGH_SPEED_ADC    
    
// !!  96 Mhz clock !!
    #if F_CPU==96000000
    {DSOADC::ADC_CAPTURE_SLOW_INTERLEAVED,"5us",     DSOADC::ADC_PRESCALER_2,  ADC_SMPR_7_5,    4096,  2400000*2}, // Seems buggy !
    {DSOADC::ADC_CAPTURE_MODE_NORMAL,"10us",     DSOADC::ADC_PRESCALER_2,  ADC_SMPR_7_5,    4096,  2400000},
    {DSOADC::ADC_CAPTURE_MODE_NORMAL,"25us",     DSOADC::ADC_PRESCALER_5,  ADC_SMPR_7_5,    4096,   960000},
    {DSOADC::ADC_CAPTURE_MODE_NORMAL,"50us",     DSOADC::ADC_PRESCALER_10, ADC_SMPR_7_5,    4096,   480000},
    {DSOADC::ADC_CAPTURE_MODE_NORMAL,"100us",    DSOADC::ADC_PRESCALER_20, ADC_SMPR_7_5,    4096,   240000},    
    {DSOADC::ADC_CAPTURE_MODE_NORMAL,"500us",    DSOADC::ADC_PRESCALER_8,  ADC_SMPR_239_5,  4063,    47619},    
    {DSOADC::ADC_CAPTURE_MODE_NORMAL,"1ms",      DSOADC::ADC_PRESCALER_16, ADC_SMPR_239_5,  4063,    23810}
   #elif F_CPU==120000000 
#error : We have to drop / duplicate too many samples at 120 Mhz, use 96!
    {false,"10us",     DSOADC::ADC_PRESCALER_2,  ADC_SMPR_7_5,    4096,  2564103},
    {false,"25us",     DSOADC::ADC_PRESCALER_5,  ADC_SMPR_7_5,    4096,  1388889},
    {false,"50us",     DSOADC::ADC_PRESCALER_10, ADC_SMPR_7_5,    4096,   529101},
    {false,"100us",    DSOADC::ADC_PRESCALER_20, ADC_SMPR_7_5,    4096,   264550},    
    {false,"500us",    DSOADC::ADC_PRESCALER_8,  ADC_SMPR_239_5,  4876,    71429},    
    {false,"1ms",      DSOADC::ADC_PRESCALER_16,  ADC_SMPR_239_5, 6502,    35714}
           
    #else
        #error unsupported MCU frequency
    #endif
    
#else // 72 Mhz clock
    {DSOADC::ADC_CAPTURE_FAST_INTERLEAVED, "5us",      DSOADC::ADC_PRESCALER_2,ADC_SMPR_1_5,   4390,   2564100*2},
    {DSOADC::ADC_CAPTURE_MODE_NORMAL,"10us",     DSOADC::ADC_PRESCALER_2,ADC_SMPR_1_5,   4390,   2564100},
    {DSOADC::ADC_CAPTURE_MODE_NORMAL,"25us",     DSOADC::ADC_PRESCALER_2,ADC_SMPR_28_5,  3747,   877193},
    {DSOADC::ADC_CAPTURE_MODE_NORMAL,"50us",     DSOADC::ADC_PRESCALER_2,ADC_SMPR_55_5,  4496,   529100},
    {DSOADC::ADC_CAPTURE_MODE_NORMAL,"100us",    DSOADC::ADC_PRESCALER_4,ADC_SMPR_55_5,  4517,   264550},    
    {DSOADC::ADC_CAPTURE_MODE_NORMAL,"500us",    DSOADC::ADC_PRESCALER_6,ADC_SMPR_239_5, 4063,   47619},    
    {DSOADC::ADC_CAPTURE_MODE_NORMAL,"1ms",      DSOADC::ADC_PRESCALER_8,ADC_SMPR_239_5, 8127,   47619}
#endif
};

/**
 */
const TimerTimeBase timerBases[]
{
    { DSOCapture::DSO_TIME_BASE_5MS,    "5ms",  4800},
    { DSOCapture::DSO_TIME_BASE_10MS,   "10ms", 2400},
    { DSOCapture::DSO_TIME_BASE_50MS,   "50ms", 480},
    { DSOCapture::DSO_TIME_BASE_100MS,  "100ms",240},
    { DSOCapture::DSO_TIME_BASE_500MS,  "500ms",48},
    { DSOCapture::DSO_TIME_BASE_1S,     "1s",   24}    
};
