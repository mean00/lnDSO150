

/*
 * Partially filled global gain array
 * Remaining columns will be filled at runtime
 */
VoltageSettings vSettings[11]=
{
    {"1mv",     1,  24000.},
    {"5mv",     2,  4800.},
    {"10mv",    3,  2400.},
    {"20mv",    4 , 1200.},
    {"50mv",    5,  480.},
    {"100mv",   6,  240.},
    {"200mv",   7,  120.},
    {"500mv",   8,  48.},
    {"1v",      9,  24.},
    {"2v",      10, 12.},
    {"5v",      11, 4.8}
};
/**
 These the time/div settings, it is computed to maximume accuracy 
 * and sample a bit too fast, so that we can decimate it
 *  */
const TimeSettings tSettings[6]
{
    {"10us",    ADC_PRE_PCLK2_DIV_2,ADC_SMPR_1_5,   4390},
    {"25us",    ADC_PRE_PCLK2_DIV_2,ADC_SMPR_13_5,  5909},
    {"50us",    ADC_PRE_PCLK2_DIV_2,ADC_SMPR_55_5,  4496},
    {"100us",   ADC_PRE_PCLK2_DIV_4,ADC_SMPR_55_5,  4517},
    {"500us",   ADC_PRE_PCLK2_DIV_4,ADC_SMPR_239_5, 6095},
    {"1ms",     ADC_PRE_PCLK2_DIV_8,ADC_SMPR_239_5, 6095}
};

/*
 * These helps when dealing with "slow" mode, i.e. when capture is controlled
 * by a timer interrupt
 */
typedef struct TimerTimeBase
{
  DSOCapture::DSO_TIME_BASE timeBase;
  const char    *name;
  int           fq;  
};
/**
 */
static const TimerTimeBase timerBases[]
{
    { DSOCapture::DSO_TIME_BASE_5MS,    "5ms",  4800},
    { DSOCapture::DSO_TIME_BASE_10MS,   "10ms", 2400},
    { DSOCapture::DSO_TIME_BASE_50MS,   "50ms", 480},
    { DSOCapture::DSO_TIME_BASE_100MS,  "100ms",240},
    { DSOCapture::DSO_TIME_BASE_500MS,  "500ms",48},
    { DSOCapture::DSO_TIME_BASE_1S,     "1s",   24}    
};
