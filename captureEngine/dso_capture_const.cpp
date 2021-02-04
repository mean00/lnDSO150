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
   

 // 72 Mhz clock
#if   F_CPU==72000000     
    {DSOADC::ADC_CAPTURE_FAST_INTERLEAVED, "5us",      DSOADC::ADC_PRESCALER_2, ADC_SMPR_1_5,   4390,   2564100*2},
    {DSOADC::ADC_CAPTURE_MODE_NORMAL,      "10us",     DSOADC::ADC_PRESCALER_2, ADC_SMPR_1_5,   4390,   2564100},
// !!  96 Mhz clock !!    
#elif F_CPU==96000000
    {DSOADC::ADC_CAPTURE_FAST_INTERLEAVED,"5us",      DSOADC::ADC_PRESCALER_2,  ADC_SMPR_1_5,   5850,  6857143 }, // Seems buggy !
    {DSOADC::ADC_CAPTURE_MODE_NORMAL,     "10us",     DSOADC::ADC_PRESCALER_2,  ADC_SMPR_7_5,   4096,  2400000},          
#elif F_CPU==128000000
    //{DSOADC::ADC_CAPTURE_MODE_NORMAL, "5us",      DSOADC::ADC_PRESCALER_2, ADC_SMPR_1_5,   3901,    4571429},
    {DSOADC::ADC_CAPTURE_FAST_INTERLEAVED,      "5us",     DSOADC::ADC_PRESCALER_4, ADC_SMPR_1_5,  1950*2,    2285714*2},
    {DSOADC::ADC_CAPTURE_MODE_NORMAL,      "10us",     DSOADC::ADC_PRESCALER_2, ADC_SMPR_13_5,  4201,    2461538},
#else
#error unsupported MCU frequency
 #endif
};

/**
 *   bool          overSampling;
  adc_smp_rate  rate ;
  DSOADC::Prescaler scale;
 */
const TimerTimeBase timerBases[]
{    
    //----------------------------------------------------------------------------------------------------------------------------------------------    
#if   F_CPU==72000000      
    //----------------------------------------------------------------------------------------------------------------------------------------------    
    { DSOCapture::DSO_TIME_BASE_25US,   "25us", 960*1000    , 1, ADC_SMPR_1_5,    DSOADC::ADC_PRESCALER_4 }, // 1 us / sample round up, error =0.3%%
    { DSOCapture::DSO_TIME_BASE_50US,   "50us", 480*1000    , 1, ADC_SMPR_1_5,    DSOADC::ADC_PRESCALER_8 }, // 2 us / sample => 0.5 with OS, no oversampling
    { DSOCapture::DSO_TIME_BASE_100US,  "100us",240*1000    , 1, ADC_SMPR_41_5,   DSOADC::ADC_PRESCALER_4 },    // 100/24= 4 us , 1 us with os
    { DSOCapture::DSO_TIME_BASE_200US,  "200us",120*1000    , 1, ADC_SMPR_239_5,  DSOADC::ADC_PRESCALER_2 },    // 100/24= 4 us , 1 us with os    
    { DSOCapture::DSO_TIME_BASE_500US,  "500us",48*1000     , 1, ADC_SMPR_239_5,  DSOADC::ADC_PRESCALER_4 },    // 500/24=> 20 us /sample => 5 us with over
    { DSOCapture::DSO_TIME_BASE_1MS,    "1ms",  24*1000     , 1, ADC_SMPR_239_5,  DSOADC::ADC_PRESCALER_8 }, // 40 us / sample => 10 us with oversampling
    { DSOCapture::DSO_TIME_BASE_2MS,    "2ms",  12*1000     , 1, ADC_SMPR_239_5,  DSOADC::ADC_PRESCALER_8 }, // 40 us / sample => 10 us with oversampling    
    { DSOCapture::DSO_TIME_BASE_5MS,    "5ms",  4800        , 1, ADC_SMPR_239_5,  DSOADC::ADC_PRESCALER_8 }, // 200 us / sample => 50 us with os
    { DSOCapture::DSO_TIME_BASE_10MS,   "10ms", 2400        , 1, ADC_SMPR_239_5,  DSOADC::ADC_PRESCALER_8 }, // 400 us / 100 us with os    **20
    { DSOCapture::DSO_TIME_BASE_20MS,   "20ms", 1200        , 1, ADC_SMPR_239_5,  DSOADC::ADC_PRESCALER_8 }, // 400 us / 100 us with os    **20    
    { DSOCapture::DSO_TIME_BASE_50MS,   "50ms", 480         , 1, ADC_SMPR_239_5,  DSOADC::ADC_PRESCALER_8 },  // 2 ms      **71
    { DSOCapture::DSO_TIME_BASE_100MS,  "100ms",240         , 1, ADC_SMPR_239_5,  DSOADC::ADC_PRESCALER_8 }, // 4 ms      **142
    { DSOCapture::DSO_TIME_BASE_200MS,  "200ms",120         , 1, ADC_SMPR_239_5,  DSOADC::ADC_PRESCALER_8 }, // 4 ms      **142    
    { DSOCapture::DSO_TIME_BASE_500MS,  "500ms",48          , 1, ADC_SMPR_239_5,  DSOADC::ADC_PRESCALER_8 },  // 20 ms     **714
    { DSOCapture::DSO_TIME_BASE_1S,     "1s",   24          , 1, ADC_SMPR_239_5,  DSOADC::ADC_PRESCALER_8 }   // 40 ms     **1428
    
 //----------------------------------------------------------------------------------------------------------------------------------------------    
    
#elif F_CPU==128000000 
 //----------------------------------------------------------------------------------------------------------------------------------------------    
    { DSOCapture::DSO_TIME_BASE_25US,   "25us", 960*1000    , 1, ADC_SMPR_1_5,   DSOADC::ADC_PRESCALER_8 }, // 1 us / sample round up, error =0.3%%
    { DSOCapture::DSO_TIME_BASE_50US,   "50us", 480*1000    , 1, ADC_SMPR_41_5,  DSOADC::ADC_PRESCALER_4 }, // 2 us / sample => 0.5 with OS, no oversampling
    { DSOCapture::DSO_TIME_BASE_100US,  "100us",240*1000    , 1, ADC_SMPR_239_5, DSOADC::ADC_PRESCALER_2 },    // 100/24= 4 us , 1 us with os
    { DSOCapture::DSO_TIME_BASE_200US,  "200us",120*1000    , 1, ADC_SMPR_239_5, DSOADC::ADC_PRESCALER_4 },    // 100/24= 4 us , 1 us with os    
    { DSOCapture::DSO_TIME_BASE_500US,  "500us",48*1000     , 1, ADC_SMPR_239_5, DSOADC::ADC_PRESCALER_8 },    // 500/24=> 20 us /sample => 5 us with over
    { DSOCapture::DSO_TIME_BASE_1MS,    "1ms",  24*1000     , 1, ADC_SMPR_239_5, DSOADC::ADC_PRESCALER_8  }, // 40 us / sample => 10 us with oversampling
    { DSOCapture::DSO_TIME_BASE_2MS,    "2ms",  12*1000     , 1, ADC_SMPR_239_5, DSOADC::ADC_PRESCALER_8  }, // 40 us / sample => 10 us with oversampling    
    { DSOCapture::DSO_TIME_BASE_5MS,    "5ms",  4800        , 1, ADC_SMPR_239_5, DSOADC::ADC_PRESCALER_8  }, // 200 us / sample => 50 us with os
    { DSOCapture::DSO_TIME_BASE_10MS,   "10ms", 2400        , 1, ADC_SMPR_239_5, DSOADC::ADC_PRESCALER_8  }, // 400 us / 100 us with os    **20
    { DSOCapture::DSO_TIME_BASE_20MS,   "20ms", 1200        , 1, ADC_SMPR_239_5, DSOADC::ADC_PRESCALER_8  }, // 400 us / 100 us with os    **20    
    { DSOCapture::DSO_TIME_BASE_50MS,   "50ms", 480         , 1, ADC_SMPR_239_5, DSOADC::ADC_PRESCALER_8  },  // 2 ms      **71
    { DSOCapture::DSO_TIME_BASE_100MS,  "100ms",240         , 1, ADC_SMPR_239_5, DSOADC::ADC_PRESCALER_8  }, // 4 ms      **142
    { DSOCapture::DSO_TIME_BASE_200MS,  "200ms",120         , 1, ADC_SMPR_239_5, DSOADC::ADC_PRESCALER_8  }, // 4 ms      **142    
    { DSOCapture::DSO_TIME_BASE_500MS,  "500ms",48          , 1, ADC_SMPR_239_5, DSOADC::ADC_PRESCALER_8  },  // 20 ms     **714
    { DSOCapture::DSO_TIME_BASE_1S,     "1s",   24          , 1, ADC_SMPR_239_5, DSOADC::ADC_PRESCALER_8  }   // 40 ms     **1428
//----------------------------------------------------------------------------------------------------------------------------------------------    
#elif F_CPU==96000000  
#ifndef GD32_ADC
    #error Only for GD32 chips!
#endif
//----------------------------------------------------------------------------------------------------------------------------------------------    
    { DSOCapture::DSO_TIME_BASE_25US,   "25us", 960*1000     , 1, ADC_SMPR_1_5,  DSOADC::ADC_PRESCALER_2 }, // 1 us / sample round up, error =0.3%%
    { DSOCapture::DSO_TIME_BASE_50US,   "50us", 480*1000     , 2, ADC_SMPR_1_5,  DSOADC::ADC_PRESCALER_4 }, // 2 us / sample => 0.5 with OS, no oversampling
    { DSOCapture::DSO_TIME_BASE_100US,  "100us",240*1000     , 4, ADC_SMPR_1_5,  DSOADC::ADC_PRESCALER_8 },    // 100/24= 4 us , 1 us with os
    { DSOCapture::DSO_TIME_BASE_200US,  "200us",120*1000     , 8, ADC_SMPR_1_5,  DSOADC::ADC_PRESCALER_8 },    // 100/24= 4 us , 1 us with os    
    { DSOCapture::DSO_TIME_BASE_500US,  "500us",48*1000      , 8, ADC_SMPR_28_5, DSOADC::ADC_PRESCALER_4 },    // 500/24=> 20 us /sample => 5 us with over
    { DSOCapture::DSO_TIME_BASE_1MS,    "1ms",  24*1000      ,16, ADC_SMPR_28_5, DSOADC::ADC_PRESCALER_5 }, // 40 us / sample => 10 us with oversampling
    { DSOCapture::DSO_TIME_BASE_2MS,    "2ms",  12*1000      ,32, ADC_SMPR_28_5, DSOADC::ADC_PRESCALER_6 }, // 40 us / sample => 10 us with oversampling    
    { DSOCapture::DSO_TIME_BASE_5MS,    "5ms",  4800        ,128, ADC_SMPR_13_5, DSOADC::ADC_PRESCALER_8 }, // 200 us / sample => 50 us with os
    { DSOCapture::DSO_TIME_BASE_10MS,   "10ms", 2400        ,256, ADC_SMPR_13_5, DSOADC::ADC_PRESCALER_16  }, // 400 us / 100 us with os    **20
    { DSOCapture::DSO_TIME_BASE_20MS,   "20ms", 1200        ,256,ADC_SMPR_13_5,  DSOADC::ADC_PRESCALER_16  }, // 400 us / 100 us with os    **20    
    { DSOCapture::DSO_TIME_BASE_50MS,   "50ms", 480         ,128,ADC_SMPR_239_5, DSOADC::ADC_PRESCALER_20  },  // 2 ms      **71
    { DSOCapture::DSO_TIME_BASE_100MS,  "100ms",240         ,256,ADC_SMPR_239_5, DSOADC::ADC_PRESCALER_20  }, // 4 ms      **142
    { DSOCapture::DSO_TIME_BASE_200MS,  "200ms",120         ,256,ADC_SMPR_239_5, DSOADC::ADC_PRESCALER_20  }, // 4 ms      **142    
    { DSOCapture::DSO_TIME_BASE_500MS,  "500ms",48          ,256,ADC_SMPR_239_5, DSOADC::ADC_PRESCALER_20  },  // 20 ms     **714
    { DSOCapture::DSO_TIME_BASE_1S,     "1s",   24          ,256,ADC_SMPR_239_5, DSOADC::ADC_PRESCALER_20  }   // 40 ms     **1428
#endif
};
