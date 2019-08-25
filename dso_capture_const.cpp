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
VoltageSettings vSettings[NB_DSO_VOLTAGE]= {
    {"1mv",     1,  24000.},
    {"2mv",     2,  24000.},
    {"5mv",     3,  4800.},
    {"10mv",    4,  2400.},
    {"20mv",    5 , 1200.},
    {"50mv",    6,  480.},
    {"100mv",   7,  240.},
    {"200mv",   8,  120.},
    {"500mv",   9,  48.},
    {"1v",      10,  24.},
    {"2v",      11, 12.},
    {"5v",      12, 4.8}
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
