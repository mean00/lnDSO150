#include "esprit.h"
//
#include "dso_control.h"
//
int counter = 0;
/**
 */
DSOControl::DSOControl(ControlEventCb *cb)
{
}
bool DSOControl::changeCb(ControlEventCb *newCb)
{
    return true;
}
bool DSOControl::setup()
{
    return true;
}
bool DSOControl::getButtonState(DSOButton button)
{
    return true;
}
int DSOControl::getButtonEvents(DSOButton button)
{
    return 0;
}
#include <raylib.h>

static int sim_rotary_val = 0;

int DSOControl::getRotaryValue()
{
    if (sim_rotary_val)
        printf(">>Rot=%d\n", sim_rotary_val);
    int v = sim_rotary_val;
    sim_rotary_val = 0;
    return v;
}
void DSOControl::interruptRE(int button)
{
}
void DSOControl::interruptButton(int button)
{
}
void DSOControl::runLoop()
{
}
extern void sim_redraw();

int DSOControl::getQButtonEvent()
{
    sim_redraw();
    if (IsKeyPressed(KEY_UP))
    {

        sim_rotary_val -= 1;
        // return DSO_EVENT_Q(DSO_BUTTON_ROTARY, EVENT_SHORT_PRESS);
    }
    if (IsKeyPressed(KEY_DOWN))
    {
        sim_rotary_val += 1;
        // return DSO_EVENT_Q(DSO_BUTTON_ROTARY, EVENT_SHORT_PRESS);
    }
    if (IsKeyPressed(KEY_ENTER))
    {
        sim_rotary_val = 0;
        return DSO_EVENT_Q(DSO_BUTTON_ROTARY, EVENT_SHORT_PRESS);
    }
    if (IsKeyPressed(KEY_BACKSPACE))
    {
        sim_rotary_val = 0;
        return DSO_EVENT_Q(DSO_BUTTON_OK, EVENT_SHORT_PRESS);
    }
    return 0;
}
void DSOControl::purgeEvent()
{
}
void DSOControl::setInputGain(int val)
{
}
DSOControl::DSOCoupling DSOControl::getCouplingState()
{
    return DSOControl::DSO_COUPLING_AC;
}
const char *DSOControl::geCouplingStateAsText()
{
    return "AC";
}
int DSOControl::getRawCoupling()
{
    return 0;
}
const char *DSOControl::getName(const DSOButton &button)
{
    return "???";
}
//--
void DSOControl::loadSettings()
{
}
void DSOControl::saveSettings()
{
}

uint32_t DSOControl::snapshot()
{
    return 0;
}
// EOF
