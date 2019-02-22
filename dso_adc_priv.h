
// this is 1/Gain for each range
// i.e. attenuation
//
const float inputScale[16]={
  /*[0] */  0, // GND
  /*[1] */  6.11/100., // /1  *14
  /*[2] */  12.19/100., // /2  *7
  /*[3] */  24.39/100., // /4  *3.5
  /*[4] */  61.64/100., // /10  *1.4
  /*[5] */  125./100., // /20  *0.8
  /*[6] */  243.9/100., // /40    *0.4 100 mv
    
  /*[7] */  6.11, // /1  /6.1   200mv
  /*[8] */  12.19, //2      /12  500mv
  /*[9] */  24.39,  //4     /24  1V
  /*[a] */  61.64,  //10     /60
  /*[b] */  125, //20     /125
  /*[c] */  250 // 40    /250
};
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
TimeSettings tSettings[6]
{
    {"10us",    ADC_PRE_PCLK2_DIV_2,ADC_SMPR_1_5,   4390},
    {"25us",    ADC_PRE_PCLK2_DIV_2,ADC_SMPR_13_5,  5909},
    {"50us",    ADC_PRE_PCLK2_DIV_2,ADC_SMPR_55_5,  4496},
    {"100us",   ADC_PRE_PCLK2_DIV_4,ADC_SMPR_55_5,  4517},
    {"500us",   ADC_PRE_PCLK2_DIV_4,ADC_SMPR_239_5, 6095},
    {"1ms",     ADC_PRE_PCLK2_DIV_8,ADC_SMPR_239_5, 6095}
};
