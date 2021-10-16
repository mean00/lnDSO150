/***************************************************
 STM32 duino based firmware for DSO SHELL/150
 *  * GPL v2
 * (c) mean 2019 fixounet@free.fr
 ****************************************************/

#include "lnArduino.h"
#include "dso_voltTime.h"
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

// EOF