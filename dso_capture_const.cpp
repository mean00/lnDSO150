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
    {"GND",     0,  24000.},
    {"5mv",     1,  4800.},
    {"10mv",    1,  2400.},
    {"20mv",    2 , 1200.},
    {"50mv",    3,  480.},
    {"100mv",   4,  240.},
    {"200mv",   5,  120.},
    {"500mv",   6,  48.},
    {"1v",      7,  24.},
    {"2v",      8, 12.},
    {"5v",      9, 4.8}
};
GainSettings gSettings[NB_ADC_VOLTAGE]=
{
    {"GND",1},
    {"20mV",8+4}, // 1
    {"40mV",8+6}, // 2
    {"80mV",8+7}, // 3
    {"200mV",8+0},// 4
    {"400mV",8+5},// 5
    {"800mV",8+3},// 6
    {"2V",0+4},   // 7
    {"4V",0+6},   // 8
    {"8V",0+7},   // 9
    {"20V",0+0},   //10
    {"40V",0+5},  // 11
    {"80V",0+3},  // 12
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
