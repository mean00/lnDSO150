/***************************************************
 STM32 duino based firmware for DSO SHELL/150
 *  * GPL v2
 * (c) mean 2019 fixounet@free.fr
 ****************************************************/

#include "dso_adc_capture.h"
 

int      DSOCapture::currentVoltageRange=0;

/**
 * 
 */
void DSOCapture::initialize()
{
}
/**
 * 
 * @param voltRange
 * @return 
 */
bool     DSOCapture::setVoltageRange(DSOCapture::DSO_VOLTAGE_RANGE voltRange)
{
    DSOCapture::currentVoltageRange=voltRange;
    DSOInputGain::setGainRange(vSettings[DSOCapture::currentVoltageRange].gain);
    return true;
}
/**
 * 
 * @return 
 */
DSOCapture::DSO_VOLTAGE_RANGE DSOCapture::getVoltageRange()
{
     return (DSOCapture::DSO_VOLTAGE_RANGE )DSOCapture::currentVoltageRange;
}

// EOF
