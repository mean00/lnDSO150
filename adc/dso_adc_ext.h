/***************************************************
 STM32 duino based firmware for DSO SHELL/150
 *  * GPL v2
 * (c) mean 2019 fixounet@free.fr
 ****************************************************/
#pragma once
#include "Arduino.h"
#include "dma.h"
#include "dso_adc_gain.h"

/* 
 * \brief Describe a voltage setting
 */

typedef struct VoltageSettings
{
    const char          *name;          /// name of the setting i.e 10 ms/div
    DSOInputGain::InputGainRange gain;
    float               displayGain;    /// multiply by this to get pixels from volt
    int                 maxSwing;
};

#include "dso_adc.h"


/**
 */
typedef struct TimeSettings
{
  DSOADC::ADC_CAPTURE_MODE   dual;
  const char         *name;
  DSOADC::Prescaler  prescaler;
  adc_smp_rate       rate;
  int                expand4096;
  int                fqInHz;
};

// EOF

