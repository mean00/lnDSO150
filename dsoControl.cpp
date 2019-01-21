// Derived from  Rotary encoder handler for arduino * Copyright 2011 Ben Buxton. Licenced under the GNU GPL Version 3. * Contact: bb@cactii.net



#include <Wire.h>
#include "dsoControl.h"
#include "dsoControl_internal.h"


static DSOControl *instance=NULL;
/**
 * \brief This one is for left/right
 * @param a
 */
static void _myInterruptRE(void *a)
{
    instance->interrupt((int)a);
}
/**
 * \brief: this one is for other buttons
 * @param a
 */
static void _myInterruptButton(void *a)
{
    
}

/**
 * 
 */
DSOControl::DSOControl()
{
      state = R_START;
      instance=this;
      counter=0;
}

#define ButtonToPin(x) (PB0+x)

/**
 * 
 * @return 
 */
bool DSOControl::setup()
{
#define pinAsInput(x) pinMode(ButtonToPin(x),INPUT_PULLUP);

#define attachRE(x)       attachInterrupt(ButtonToPin(x),_myInterruptRE,(void *)x,FALLING );
#define attachButton(x)   attachInterrupt(ButtonToPin(x),_myInterruptButton,(void *)x,CHANGE);
    
    
    pinAsInput(DSO_BUTTON_UP);
    pinAsInput(DSO_BUTTON_DOWN);
    
    pinAsInput(DSO_BUTTON_ROTARY);
    pinAsInput(DSO_BUTTON_VOLTAGE);
    pinAsInput(DSO_BUTTON_TIME);
    pinAsInput(DSO_BUTTON_TRIGGER);
    pinAsInput(DSO_BUTTON_OK);
      
    
    
    attachRE(DSO_BUTTON_UP);
    attachRE(DSO_BUTTON_DOWN);
    
    attachButton(DSO_BUTTON_ROTARY);
    attachButton(DSO_BUTTON_VOLTAGE);
    attachButton(DSO_BUTTON_TIME);
    attachButton(DSO_BUTTON_TRIGGER);
    attachButton(DSO_BUTTON_OK);
}
/**
 * 
 * @param a
 */
void DSOControl::interrupt(int a)
{
   
  // Grab state of input pins.
  unsigned char pinstate = (((!a) << 1) | (a&1));
  // Determine new state from the pins and state table.
  state = ttable[state & 0xf][pinstate];
  // Return emit bits, ie the generated event.
  if(state==DIR_CW) counter++;
  if(state==DIR_CCW) counter--;
  
}
/**
 * 
 * @param button
 * @return 
 */
bool DSOControl::getButtonState(DSOControl::DSOButton button)
{
        uint32_t val= GPIOB->regs->IDR;      
        return !(val&1<<button); // this is crude
}
/**
 * 
 * @return 
 */
int  DSOControl::getRotaryValue()
{
    int r;
    noInterrupts();
    r=counter;
    counter=0;
    interrupts();
    return r;
}
//