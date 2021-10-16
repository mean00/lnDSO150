#pragma once


#include "dso_adc_gain.h"


struct VoltageSettings
{
     const char          *name;          /// name of the setting i.e 10 ms/div
     int gain; //DSOInputGain::InputGainRange gain;
     float               displayGain;    /// multiply by this to get pixels from volt
     int                 maxSwing;
};
#define NB_CAPTURE_VOLTAGE  11
extern VoltageSettings      vSettings[NB_CAPTURE_VOLTAGE];
