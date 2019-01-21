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
 * 
 * @param a
 */
static void _myInterrupt(void *a)
{
    instance->interrupt((int)a);
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
    pinMode(PB0,INPUT_PULLUP);
    pinMode(PB1,INPUT_PULLUP);
    attachInterrupt(PB0,_myInterrupt,(void *)0,FALLING);  
    attachInterrupt(PB1,_myInterrupt,(void *)1,FALLING);  
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