
#include "esprit.h"
//-----------------------------------------------
#include "DSO_portBArbitrer.h"
uint32_t _directionPort[2] = {0, 0};
uint32_t _valuePort[2] = {0, 0};
DSO_portArbitrer arbitrer(0, NULL);
DSO_portArbitrer::DSO_portArbitrer(int port, lnMutex *tex)
{
}
void DSO_portArbitrer::setInputDirectionValue(uint32_t v, uint32_t v2)
{
}
void DSO_portArbitrer::setInputValue(uint32_t v)
{
}
void DSO_portArbitrer::beginInput()
{
}
void DSO_portArbitrer::endInput()
{
}
void DSO_portArbitrer::beginLCD()
{
}
void DSO_portArbitrer::endLCD()
{
}
//-----------------------------------------------
#include "dso_calibrate.h"
//-----------------------------------------------
bool DSOCalibrate::loadCalibrationData()
{
    return true;
}
bool DSOCalibrate::zeroCalibrate()
{
    return true;
}
bool DSOCalibrate::decalibrate()
{
    return true;
}
bool DSOCalibrate::voltageCalibrate()
{
    return true;
}
bool DSOCalibrate::vccAdcMenu()
{
    return true;
}
bool DSOCalibrate::fiveVoltCalibrate()
{
    return true;
}
bool DSOCalibrate::formatNvm()
{
    return true;
}
bool DSOCalibrate::voltageCalibrate_()
{
    return true;
}
bool DSOCalibrate::zeroCalibrate_()
{
    return true;
}
bool DSOCalibrate::fiveVoltCalibrate_()
{
    return true;
}
//--------------------------------------
const char *getScreenID()
{
    return "?????";
}

// EOF
