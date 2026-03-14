
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
#include "dso_control.h"
//-----------------------------------------------
bool DSOControl::changeCb(ControlEventCb *newCb)
{
    return true;
}
int DSOControl::getRotaryValue()
{
    return 0;
}
int DSOControl::getQButtonEvent()
{
    return 0;
}
void DSOControl::purgeEvent()
{
}
bool DSOControl::getButtonState(DSOControl::DSOButton button)
{
    return false;
}
// EOF
