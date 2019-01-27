
/**
 Pin usage
 * 
 * PC13/14/15 OUTPUT LCD control 
 * PB0--PB7   OUTPUT LCD Databus
 * PA0        x      ADCIN
 * PA6        OUTPUT LCD nRD
 * PA7        OUTPUT Test signal
 * PA8        INPUT  Trig
 * PB0--B1    INPUT Rotary encoder
 * PB3--B7    INPUT Buttons
 * 
 * PB8        x      TL_PWM
 * PB9        OUTPUT LCD RESET
 * PB12       OUTPUT AMPSEL
 * 
 
 * PA1..A4    x      SENSEL
 * PA5        INPUT  CPLSEL
 * 
 
 
 */
// Rotary encoder part : Derived from  Rotary encoder handler for arduino * Copyright 2011 Ben Buxton. Licenced under the GNU GPL Version 3. * Contact: bb@cactii.net
// Debounce part derived from http://www.kennethkuhn.com/electronics/debounce.c


#include <Wire.h>
#include "MapleFreeRTOS1000_pp.h"
#include "dsoControl.h"
#include "dsoControl_internal.h"

#define TICK             10
#define LONG_PRESS_THRESHOLD (1000/TICK)
#define SHORT_PRESS_THRESHOLD (2)

 enum DSOButtonState
  {
    StateIdle=0,
    StatePressed=1,
    StateLongPressed=2
  };
#define ButtonToPin(x) (PB0+x)
#define pinAsInput(x)     pinMode(ButtonToPin(x),INPUT_PULLUP);
#define attachRE(x)       attachInterrupt(ButtonToPin(x),_myInterruptRE,(void *)x,FALLING );

#define NB_BUTTONS 8

static DSOControl *instance=NULL;
static int          pinState[NB_BUTTONS];
static int          pinCount[NB_BUTTONS];
static int          events[NB_BUTTONS];
static int          states[NB_BUTTONS];

static int state;  // rotary state
static int counter; // rotary counter

static TaskHandle_t taskHandle;

#define COUNT_MAX 3


/**
 * \brief This one is for left/right
 * @param a
 */
static void _myInterruptRE(void *a)
{
    instance->interruptRE(!!a);
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
    
    for(int i=0;i<NB_BUTTONS;i++)
    {
        pinState[i]=0;
        pinCount[i]=0;
        states[i]=StateIdle;
    }
    
}


static void trampoline(void *a)
{
    DSOControl *ctrl=(DSOControl*)a;
    ctrl->runLoop();
}
/**
 * 
 */
void DSOControl::runLoop()
{
    xDelay(5);
    while(1)
    {
        xDelay(TICK);
        uint32_t val= GPIOB->regs->IDR;      
        for(int i=DSO_BUTTON_ROTARY;i<=DSO_BUTTON_OK;i++)
        {
            int k=!(val&(1<<i));
            int *c=pinCount+i;
            int oldCount=*c;
            if(k)
            {
                (*c)++;
            }else
            {
                if(*c) 
                {
                    if(*c>=COUNT_MAX) *c=COUNT_MAX-1;
                    else
                        (*c)--;
                }
            }
            int oldPin=pinState[i];
            int newPin=(*c)>(COUNT_MAX-1);
            
            int s=oldPin+oldPin+newPin;
            switch(s)
            {
                default:
                case 0:
                    break;
                case 2:
                { // released
                    if(states[i]==StatePressed)
                    {
                        states[i]=StateIdle;
                        if(oldCount>SHORT_PRESS_THRESHOLD)
                        {
                            events[i]=EVENT_SHORT_PRESS;
                            Serial.print("Short");
                            Serial.println(i);
                        }
                    }
                    states[i]=StateIdle;
                    break;
                }
                case 1: // Pressed
                    states[i]=StatePressed;
                    break;
                case 3: // Still pressed
                    if(*c>LONG_PRESS_THRESHOLD && states[i]==StatePressed) // only one long
                    {
                        states[i]=StateLongPressed;
                        events[i]=EVENT_LONG_PRESS;
                        Serial.print("Long");
                        Serial.println(i);
                    }
                    break;
            }                       
            pinState[i]=newPin;
        }        
    }
}

/**
 * 
 * @return 
 */
bool DSOControl::setup()
{
    attachRE(DSO_BUTTON_UP);
    attachRE(DSO_BUTTON_DOWN);
    xTaskCreate( trampoline, "Control", 150, this, 15, &taskHandle );       
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
    return pinState[button];
}
/**
 * 
 * @param button
 * @return 
 */
int  DSOControl::getButtonEvents(DSOButton button)
{
    int evt=events[button];
    events[button]=0;
    return evt;
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