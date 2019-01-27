#pragma once


#define EVENT_LONG_PRESS  1
#define EVENT_SHORT_PRESS 2

/**
 */
class DSOControl
{
public:
 
  enum DSOButton
  {
    DSO_BUTTON_UP=0,
    DSO_BUTTON_DOWN=1,
    DSO_BUTTON_ROTARY=3,
    DSO_BUTTON_VOLTAGE=4,
    DSO_BUTTON_TIME=5,
    DSO_BUTTON_TRIGGER=6,
    DSO_BUTTON_OK=7
  };

         DSOControl();
    bool setup();
    bool getButtonState(DSOButton button);
    int  getButtonEvent(DSOButton button);
    int  getRotaryValue();
    void interruptRE(int button);
    void interruptButton(int button);
    void runLoop();
protected:
    
};
// EOF
