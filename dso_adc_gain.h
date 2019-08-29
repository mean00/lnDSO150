/**
 
 
 
 */
#pragma once

#define DSO_NB_GAIN_RANGES 13
/**
 */
class DSOInputGain
{
public:
    enum InputGainRange
    {
      MAX_VOLTAGE_GND=0,    // 0
      MAX_VOLTAGE_20MV,     // 1
      MAX_VOLTAGE_40MV,     // 2
      MAX_VOLTAGE_80MV,     // 3
      MAX_VOLTAGE_200MV,    // 4
      MAX_VOLTAGE_400MV,    // 5
      MAX_VOLTAGE_800MV,    // 6
      MAX_VOLTAGE_2MV,      // 7
      MAX_VOLTAGE_4MV,      // 8
      MAX_VOLTAGE_8MV,      // 9
      MAX_VOLTAGE_20V,     // 10
      MAX_VOLTAGE_40V,     // 11
      MAX_VOLTAGE_80V=12   // 12
    };
    static bool                         setGainRange(InputGainRange range);
    static DSOInputGain::InputGainRange getGainRange();
    static int                          getOffset(int dc0ac1);
    static float                        getMultiplier();
    static bool                         readCalibrationValue();
};