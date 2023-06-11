/***************************************************
 STM32 duino based firmware for DSO SHELL/150
 *  * GPL v2
 * (c) mean 2019 fixounet@free.fr
 ****************************************************/
#pragma once

#define EVENT_LONG_PRESS 1
#define EVENT_SHORT_PRESS 2

/**
 */
class DSOControl
{
  public:
    enum DSOEvent
    {

        DSOEventControl,
        DSOEventCoupling
    };

#define DSO_EVENT_Q(b, e) ((b + (e << 16)))
    enum DSOButton
    {
        DSO_BUTTON_UP = 0,
        DSO_BUTTON_DOWN = 1,
        DSO_BUTTON_ROTARY = 2,
        DSO_BUTTON_VOLTAGE = 3,
        DSO_BUTTON_TIME = 4,
        DSO_BUTTON_TRIGGER = 5,
        DSO_BUTTON_OK = 6
    };

// V/DIV    => B4
// SEC/DIV  => B5
// TRIGGER  => B6
// OK       => B7
// Rotary   => B3 or B8
#define DSO_CONTROL_BUTTON_PORT_A 32
#ifdef USE_FNIRSI_BUTTON
    const int ButtonMapping[7] = {0, 1, 3, 4, 5, DSO_CONTROL_BUTTON_PORT_A + 12, DSO_CONTROL_BUTTON_PORT_A + 11};
#else
#ifndef USE_PB8_INSTEAD_OF_PB3
    const int ButtonMapping[7] = {0, 1, 3, 4, 5, 6, 7};
#else
    const int ButtonMapping[7] = {0, 1, 8, 4, 5, 6, 7};
#endif
#endif

    enum DSOButtonState
    {
        StateIdle = 0,
        StatePressed = 1,
        StateLongPressed = 2,
        StateHoldOff = 3
    };
    enum DSOCoupling
    {
        DSO_COUPLING_GND = 0,
        DSO_COUPLING_DC = 1,
        DSO_COUPLING_AC = 2
    };
    typedef void ControlEventCb(DSOControl::DSOEvent evt);

    DSOControl(ControlEventCb *cb);
    bool changeCb(ControlEventCb *newCb);
    ControlEventCb *getCb()
    {
        return _cb;
    }
    bool setup();
    bool getButtonState(DSOButton button);
    int getButtonEvents(DSOButton button);
    int getRotaryValue();
    void interruptRE(int button);
    void interruptButton(int button);
    void runLoop();
    int getQButtonEvent(); // return 0 if nothing to do, else key + event<<16
    void purgeEvent();
    void setInputGain(int val); // This drives SENSEL... Warning the mapping is not straightforward !
    DSOCoupling getCouplingState();
    const char *geCouplingStateAsText();
    int getRawCoupling();
    void invert(bool inv)
    {
        _inverted = inv;
    }
    bool inverted()
    {
        return _inverted;
    }
    static const char *getName(const DSOButton &button);
    //--
    void loadSettings();
    void saveSettings();

  protected:
    uint32_t snapshot();
    int couplingValue;
    DSOCoupling couplingState;
    ControlEventCb *_cb;
    bool _inverted;
};
// EOF
