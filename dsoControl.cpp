// Derived from  Rotary encoder handler for arduino * Copyright 2011 Ben Buxton. Licenced under the GNU GPL Version 3. * Contact: bb@cactii.net



#include <Wire.h>
#include "dsoControl.h"

#define DIR_CW 0x10
#define DIR_CCW 0x20

#define R_START 0x0
#define R_CW_FINAL 0x1
#define R_CW_BEGIN 0x2
#define R_CW_NEXT 0x3
#define R_CCW_BEGIN 0x4
#define R_CCW_FINAL 0x5
#define R_CCW_NEXT 0x6

const unsigned char ttable[7][4] = {
  // R_START
  {R_START,    R_CW_BEGIN,  R_CCW_BEGIN, R_START},
  // R_CW_FINAL
  {R_CW_NEXT,  R_START,     R_CW_FINAL,  R_START | DIR_CW},
  // R_CW_BEGIN
  {R_CW_NEXT,  R_CW_BEGIN,  R_START,     R_START},
  // R_CW_NEXT
  {R_CW_NEXT,  R_CW_BEGIN,  R_CW_FINAL,  R_START},
  // R_CCW_BEGIN
  {R_CCW_NEXT, R_START,     R_CCW_BEGIN, R_START},
  // R_CCW_FINAL
  {R_CCW_NEXT, R_CCW_FINAL, R_START,     R_START | DIR_CCW},
  // R_CCW_NEXT
  {R_CCW_NEXT, R_CCW_FINAL, R_CCW_BEGIN, R_START},
};

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
/**
 * 
 * @return 
 */
bool DSOControl::setup()
{
#define pinAsInput(x) pinMode(PB0+x,INPUT_PULLUP);
    
    
    pinAsInput(DSO_BUTTON_UP);
    pinAsInput(DSO_BUTTON_DOWN);
    
    pinAsInput(DSO_BUTTON_ROTARY);
    pinAsInput(DSO_BUTTON_VOLTAGE);
    pinAsInput(DSO_BUTTON_TIME);
    pinAsInput(DSO_BUTTON_TRIGGER);
    pinAsInput(DSO_BUTTON_OK);
      
    
#define attachRE(x)       attachInterrupt((uint8_t)(PB0+x),_myInterruptRE,(void *)x,FALLING );
#define attachButton(x)   attachInterrupt((uint8_t)(PB0+x),_myInterruptButton,(void *)x,CHANGE);
    
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
  unsigned char pinstate = (((a==0) << 1) | (a==1));
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