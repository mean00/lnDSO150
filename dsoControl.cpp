// Derived from  Rotary encoder handler for arduino * Copyright 2011 Ben Buxton. Licenced under the GNU GPL Version 3. * Contact: bb@cactii.net



#include <Wire.h>
#include "dsoControl.h"
#include "dsoControl_internal.h"
#define ButtonToPin(x) (PB0+x)
#define pinAsInput(x)     pinMode(ButtonToPin(x),INPUT_PULLUP);
#define attachRE(x)       attachInterrupt(ButtonToPin(x),_myInterruptRE,(void *)x,FALLING );
#define attachButton(x)   attachInterrupt(ButtonToPin(x),_myInterruptButton,(void *)x,CHANGE);


static DSOControl *instance=NULL;
/**
 * \brief This one is for left/right
 * @param a
 */
static void _myInterruptRE(void *a)
{
    instance->interruptRE(!!a);
}
/**
 * \brief: this one is for other buttons
 * @param a
 */
static void _myInterruptButton(void *a)
{
    instance->interruptButton((int)a);
}

/**
 * 
 */
DSOControl::DSOControl()
{
    state = R_START;
    instance=this;
    counter=0;
    pinAsInput(DSO_BUTTON_UP);
    pinAsInput(DSO_BUTTON_DOWN);
    
    pinAsInput(DSO_BUTTON_ROTARY);
    pinAsInput(DSO_BUTTON_VOLTAGE);
    pinAsInput(DSO_BUTTON_TIME);
    pinAsInput(DSO_BUTTON_TRIGGER);
    pinAsInput(DSO_BUTTON_OK);
}



/**
 * 
 * @return 
 */
bool DSOControl::setup()
{
    attachRE(DSO_BUTTON_UP);
    attachRE(DSO_BUTTON_DOWN);
/*    
    attachButton(DSO_BUTTON_ROTARY);
    attachButton(DSO_BUTTON_VOLTAGE);
    attachButton(DSO_BUTTON_TIME);
    attachButton(DSO_BUTTON_TRIGGER);
    attachButton(DSO_BUTTON_OK);
 * */
}
/**
 * 
 * @param a
 */
void DSOControl::interruptButton(int a)
{
    
}

/**
 * 
 * @param a
 */
void DSOControl::interruptRE(int a)
{
   
  // Grab state of input pins.
  int pinstate =  ( GPIOB->regs->IDR)&3;
  // Determine new state from the pins and state table.
  state = ttable[state & 0xf][pinstate];
  // Return emit bits, ie the generated event.
  switch(state&DIR_MASK)
  {
    case DIR_CW:
            counter--;
            break;
    case DIR_CCW: 
            counter++;
            break;
    default: 
            break;
  }
  
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