
#include "lnArduino.h"
#include "dso_adc_capture.h"


/*
 * Partially filled global gain array
 * Remaining columns will be filled at runtime
 */
#define SAT_THRESHOLD 66 //5%
#define SWING(x) (2048-x+SAT_THRESHOLD)


VoltageSettings vSettings[NB_CAPTURE_VOLTAGE]= {
    {"GND",     DSOInputGain::MAX_VOLTAGE_GND,  20000.,    SWING(2048) },
    {"5mv",     DSOInputGain::MAX_VOLTAGE_20MV,  4000.,    SWING(2048)}, // "Digital" zoom
    {"10mv",    DSOInputGain::MAX_VOLTAGE_20MV,  2000.,    SWING(2048)},
    {"20mv",    DSOInputGain::MAX_VOLTAGE_40MV , 1000.,    SWING(2048)},
    {"50mv",    DSOInputGain::MAX_VOLTAGE_80MV,  400.,     SWING(2048)},
    {"100mv",   DSOInputGain::MAX_VOLTAGE_200MV,  200.,    SWING(2048)},
    {"200mv",   DSOInputGain::MAX_VOLTAGE_250MV,  100.,    SWING(1100)}, // that one will saturate early
    //{"500mv",   DSOInputGain::MAX_VOLTAGE_800MV,  48.},
    {"500mV",   DSOInputGain::MAX_VOLTAGE_2V,      40.,    SWING(2048)}, // "digital" zoom
    {"1v",      DSOInputGain::MAX_VOLTAGE_2V,      20.,    SWING(2048)},
    {"2v",      DSOInputGain::MAX_VOLTAGE_4V,      10.,    SWING(2048)},
    {"5v",      DSOInputGain::MAX_VOLTAGE_8V,       4.,    SWING(2048)}
};

#define DSO_FQ_US(x) (20*1000*1000/x)
const TimerTimeBase timerBases[]=
{   //                                  20 pix/dev => fq*20
    { DSOCapture::DSO_TIME_BASE_10US,   "10us",  DSO_FQ_US(10)   }, // 1 us / sample round up, error =0.3%%
    { DSOCapture::DSO_TIME_BASE_20US,   "20us",  DSO_FQ_US(20)   }, // 1 us / sample round up, error =0.3%%
    //---
    { DSOCapture::DSO_TIME_BASE_50US,   "50us",  DSO_FQ_US(50)   }, // 2 us / sample => 0.5 with OS, no oversampling
    { DSOCapture::DSO_TIME_BASE_100US,  "100us", DSO_FQ_US(100)  },    // 100/24= 4 us , 1 us with os
    { DSOCapture::DSO_TIME_BASE_200US,  "200us", DSO_FQ_US(200)  },    // 100/24= 4 us , 1 us with os    
    { DSOCapture::DSO_TIME_BASE_500US,  "500us", DSO_FQ_US(500)  },    // 500/24=> 20 us /sample => 5 us with over
    { DSOCapture::DSO_TIME_BASE_1MS,    "1ms",   DSO_FQ_US(1000) }, // 40 us / sample => 10 us with oversampling
    { DSOCapture::DSO_TIME_BASE_2MS,    "2ms",   DSO_FQ_US(2000) }, // 40 us / sample => 10 us with oversampling    
    { DSOCapture::DSO_TIME_BASE_5MS,    "5ms",   DSO_FQ_US(5000) }, // 200 us / sample => 50 us with os
    { DSOCapture::DSO_TIME_BASE_10MS,   "10ms",  DSO_FQ_US(10000)}, // 400 us / 100 us with os    **20
    { DSOCapture::DSO_TIME_BASE_20MS,   "20ms",  DSO_FQ_US(20000)}, // 400 us / 100 us with os    **20    
    { DSOCapture::DSO_TIME_BASE_50MS,   "50ms",  DSO_FQ_US(50000)},  // 2 ms      **71
 };
/**
 */
TimerTimeADC timerADC[]=
{   //                                 OVER    CYCLES                 PREDIV 
    { DSOCapture::DSO_TIME_BASE_10US,   1, LN_ADC_SMPT_1_5,    lnADC_CLOCK_DIV_BY_2 }, // 1 us / sample round up, error =0.3%%
    { DSOCapture::DSO_TIME_BASE_20US,   1, LN_ADC_SMPT_1_5,    lnADC_CLOCK_DIV_BY_2 }, // 1 us / sample round up, error =0.3%%
    //---
    { DSOCapture::DSO_TIME_BASE_50US,   1, LN_ADC_SMPT_1_5,    lnADC_CLOCK_DIV_BY_8 }, // 2 us / sample => 0.5 with OS, no oversampling
    { DSOCapture::DSO_TIME_BASE_100US,  1, LN_ADC_SMPT_41_5,   lnADC_CLOCK_DIV_BY_4 },    // 100/24= 4 us , 1 us with os
    { DSOCapture::DSO_TIME_BASE_200US,  1, LN_ADC_SMPT_239_5,  lnADC_CLOCK_DIV_BY_2 },    // 100/24= 4 us , 1 us with os    
    { DSOCapture::DSO_TIME_BASE_500US,  1, LN_ADC_SMPT_239_5,  lnADC_CLOCK_DIV_BY_4 },    // 500/24=> 20 us /sample => 5 us with over
    { DSOCapture::DSO_TIME_BASE_1MS,    1, LN_ADC_SMPT_239_5,  lnADC_CLOCK_DIV_BY_8 }, // 40 us / sample => 10 us with oversampling
    { DSOCapture::DSO_TIME_BASE_2MS,    1, LN_ADC_SMPT_239_5,  lnADC_CLOCK_DIV_BY_8 }, // 40 us / sample => 10 us with oversampling    
    { DSOCapture::DSO_TIME_BASE_5MS,    1, LN_ADC_SMPT_239_5,  lnADC_CLOCK_DIV_BY_8 }, // 200 us / sample => 50 us with os
    { DSOCapture::DSO_TIME_BASE_10MS,   1, LN_ADC_SMPT_239_5,  lnADC_CLOCK_DIV_BY_8 }, // 400 us / 100 us with os    **20
    { DSOCapture::DSO_TIME_BASE_20MS,   1, LN_ADC_SMPT_239_5,  lnADC_CLOCK_DIV_BY_8 }, // 400 us / 100 us with os    **20    
    { DSOCapture::DSO_TIME_BASE_50MS,   1, LN_ADC_SMPT_239_5,  lnADC_CLOCK_DIV_BY_8 },  // 2 ms      **71
 };

