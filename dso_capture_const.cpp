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
VoltageSettings vSettings[NB_CAPTURE_VOLTAGE]= {
    {"GND",     DSOInputGain::MAX_VOLTAGE_GND,  24000.},
    {"5mv",     DSOInputGain::MAX_VOLTAGE_20MV,  4800.}, // "Digital" zoom
    {"10mv",    DSOInputGain::MAX_VOLTAGE_20MV,  2400.},
    {"20mv",    DSOInputGain::MAX_VOLTAGE_40MV , 1200.},
    {"50mv",    DSOInputGain::MAX_VOLTAGE_80MV,  480.},
    {"100mv",   DSOInputGain::MAX_VOLTAGE_200MV,  240.},
    {"200mv",   DSOInputGain::MAX_VOLTAGE_250MV,  120.},
    //{"500mv",   DSOInputGain::MAX_VOLTAGE_800MV,  48.},
    {"500mV",   DSOInputGain::MAX_VOLTAGE_2V,  48.}, // "digital" zoom
    {"1v",      DSOInputGain::MAX_VOLTAGE_2V,  24.},
    {"2v",      DSOInputGain::MAX_VOLTAGE_4V, 12.},
    {"5v",      DSOInputGain::MAX_VOLTAGE_8V, 4.8}
};


/**
 These the time/div settings, it is computed to maximume accuracy 
 * and sample a bit too fast, so that we can decimate it
 *  */
const TimeSettings tSettings[6]
{
    {"10us",    ADC_PRE_PCLK2_DIV_2,ADC_SMPR_1_5,   4390,   2564100},
    //{"25us",    ADC_PRE_PCLK2_DIV_2,ADC_SMPR_13_5,  5909,   1388890},
    {"25us",    ADC_PRE_PCLK2_DIV_2,ADC_SMPR_28_5,  3747,   877193},
    {"50us",    ADC_PRE_PCLK2_DIV_2,ADC_SMPR_55_5,  4496,   529100},
    {"100us",   ADC_PRE_PCLK2_DIV_4,ADC_SMPR_55_5,  4517,   264550},
    //{"500us",   ADC_PRE_PCLK2_DIV_4,ADC_SMPR_239_5, 6095,   71430},
    {"500us",   ADC_PRE_PCLK2_DIV_6,ADC_SMPR_239_5, 4063,   47619},    
    //{"1ms",     ADC_PRE_PCLK2_DIV_8,ADC_SMPR_239_5, 6095,   35710}
    {"1ms",     ADC_PRE_PCLK2_DIV_6,ADC_SMPR_239_5, 8127,   47619}
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
