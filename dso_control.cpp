
/**
 Pin usage
 * 
 * PC13/14/15 OUTPUT LCD control 
 * PB0--PB7   OUTPUT LCD Databus
 * PA0        x      ADCIN
 * PA1..PA3   x      Gain 2nd stage SENSEL0..SENSEL2
 * PA4        x      Gain 1st stage SENSEL3
 * PA5        x      CPLSEL (DC/AC/GND)
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
 * PA9/PA10   INPUT Uart RX/TX used for rotary encoder
 * PA5        INPUT  CPLSEL
 * 
 
 STM32duino info here : http://wiki.stm32duino.com/index.php?title=Generic_pin_mapping
 */
// Rotary encoder part : Derived from  Rotary encoder handler for arduino * Copyright 2011 Ben Buxton. Licenced under the GNU GPL Version 3. * Contact: bb@cactii.net
// Debounce part derived from http://www.kennethkuhn.com/electronics/debounce.c


#include <Wire.h>
#include "MapleFreeRTOS1000_pp.h"
#include "dso_control.h"
#include "dso_control_internal.h"
#include "DSO_config.h"
#include "fancyLock.h"
#define TICK                  10 // 10 ms
#define LONG_PRESS_THRESHOLD (1000/TICK) // 1s
#define SHORT_PRESS_THRESHOLD (3) // 20 ms
#define HOLDOFF_THRESHOLD     (100/TICK)
#define COUNT_MAX             4

extern uint16_t directADC2Read(int pin);

int debugUp=0;
int debugDown=0;

int ampMapping[16]=
{
    1 , // GND          [0]
    
    8+4, // x14   1mv   [1]
    8+6, // x7           [2]
    8+7, // x3.5        [3]
    8+0, // x1.4        [4]
    8+5, // x0.7        [5]
    8+3, // x0.35       [6]
    
    4, // /7    100 mv  [7]
    6, // /14   200 mv  [8]
    7, // /29   500 mv  [9]
    0, // /71   1v      [10]
    5, // /143  2v      [11]
    3,  // /286 5v      [12]
    
    3,3,3 // Filler
};


 enum DSOButtonState
  {
    StateIdle=0,
    StatePressed=1,
    StateLongPressed=2,
    StateHoldOff=3
  };
#define ButtonToPin(x)    (PB0+x)
#define pinAsInput(x)     pinMode(ButtonToPin(x),INPUT_PULLUP);
#define attachRE(x)       attachInterrupt(ButtonToPin(x),_myInterruptRE,(void *)x,FALLING );

#define NB_BUTTONS 8

#if 0 // Use TX/RX pin for rotary encoder
#define ALT_ROTARY_LEFT   PA9
#define ALT_ROTARY_RIGHT  PA10
#define ROTARY_GPIO       GPIOA
#define ROTATY_SHIFT      9  
#else  // Use Pb14 & PB15 for rotary encoder
#define ALT_ROTARY_LEFT   PB14
#define ALT_ROTARY_RIGHT  PB15
#define ROTARY_GPIO       GPIOB  
#define ROTATY_SHIFT      14  
#endif

#define COUPLING_PIN PA5
  
#define SENSEL_PIN PA1 //(1..4)
  
extern FancyLock PortAMutex; // lock against LCD  
static int rawCoupling;  
/**
 */
class singleButton
{
public:
                    singleButton()
                    {
                        _state=StateIdle;
                        _events=0;
                        _holdOffCounter=0;
                        _pinState=0;
                        _pinCounter=0;
                    }
                    bool holdOff() // Return true if in holdoff mode
                    {
                        if(_state!=StateHoldOff)
                            return false;
                        _holdOffCounter++;
                        if(_holdOffCounter>HOLDOFF_THRESHOLD)
                        {
                            _state=StateIdle;
                            return false;
                        }
                        return true;
                    }
                    void goToHoldOff()
                    {
                        _state=StateHoldOff;
                        _holdOffCounter=0;
                    }
                    void integrate(bool k)
                    {         
                        // Integrator part
                        if(k)
                        {
                            _pinCounter++;
                        }else
                        {
                            if(_pinCounter) 
                            {
                                if(_pinCounter>COUNT_MAX) _pinCounter=COUNT_MAX;
                                else
                                    _pinCounter--;
                            }
                        }
                    }
                    
                    void runMachine(int oldCount)
                    {
                        
                        int oldPin=_pinState;
                        int newPin=_pinCounter>(COUNT_MAX-1);

                        int s=oldPin+oldPin+newPin;
                        switch(s)
                        {
                            default:
                            case 0: // flat
                                break;
                            case 2:
                            { // released
                                if(_state==StatePressed)
                                {                        
                                    if(oldCount>SHORT_PRESS_THRESHOLD)
                                    {
                                        _events|=EVENT_SHORT_PRESS;
                                    }
                                }
                                goToHoldOff();
                                break;
                            }
                            case 1: // Pressed
                                _state=StatePressed;
                                break;
                            case 3: // Still pressed
                                if(_pinCounter>LONG_PRESS_THRESHOLD && _state==StatePressed) // only one long
                                {
                                    _state=StateLongPressed;
                                    _events|=EVENT_LONG_PRESS;                                    
                                }
                                break;
                        }                       
                        _pinState=newPin;
                    }        
                    
    DSOButtonState _state;
    int            _events;
    int            _holdOffCounter;
    int            _pinState;
    int            _pinCounter;
};
  
static DSOControl  *instance=NULL;

static singleButton _buttons[NB_BUTTONS];

static int state;  // rotary state
static int counter; // rotary counter

static TaskHandle_t taskHandle;

int ints=0;
/**
 * \brief This one is for left/right
 * @param a
 */
static void _myInterruptRE(void *a)
{
    ints++;
    instance->interruptRE(!!a);
}
/**
 * 
 * @param button
 * @return 
 */
const char *DSOControl::getName(const DSOButton &button)
{
#define XBUT(x)    case DSO_BUTTON_##x: return #x;break;
    switch(button)
    {
    XBUT(UP)
    XBUT(DOWN)
    XBUT(ROTARY)
    XBUT(VOLTAGE)
    XBUT(TIME)
    XBUT(TRIGGER)
    XBUT(OK)
    }
    return "???";    
}
/**
 * 
 * @return 
 */
int DSOControl::getRawCoupling()
{
    return rawCoupling; 
}
 
/**
 * 
 * @param v
 * @return 
 */
static DSOControl::DSOCoupling couplingFromAdc2()
{
    rawCoupling= directADC2Read(COUPLING_PIN);    
    if(rawCoupling>3200)      
        return DSOControl::DSO_COUPLING_AC;
    if(rawCoupling<1000)       
        return DSOControl::DSO_COUPLING_GND;
    return DSOControl::DSO_COUPLING_DC;
}
/**
 * 
 */
DSOControl::DSOControl()
{
    state = R_START;
    instance=this;
    counter=0;
    
#ifdef USE_RXTX_PIN_FOR_ROTARY
    pinMode(ALT_ROTARY_LEFT,OUTPUT); // ok
    digitalWrite(ALT_ROTARY_LEFT,1);
    
    pinMode(ALT_ROTARY_RIGHT,OUTPUT); // ok
    digitalWrite(ALT_ROTARY_RIGHT,1);    
    
    pinMode(ALT_ROTARY_LEFT,INPUT_PULLUP); // ok
    pinMode(ALT_ROTARY_RIGHT,INPUT_PULLUP);
#else    
    pinAsInput(DSO_BUTTON_UP);
    pinAsInput(DSO_BUTTON_DOWN);
#endif
    
    pinAsInput(DSO_BUTTON_ROTARY);
    pinAsInput(DSO_BUTTON_VOLTAGE);
    pinAsInput(DSO_BUTTON_TIME);
    pinAsInput(DSO_BUTTON_TRIGGER);
    pinAsInput(DSO_BUTTON_OK);
    
    for(int i=0;i<4;i++)    
        pinMode(SENSEL_PIN+i,OUTPUT); // SENSEL
    
    pinMode(COUPLING_PIN,INPUT_ANALOG);
    couplingState=couplingFromAdc2();
    
}


static void trampoline(void *a)
{
    DSOControl *ctrl=(DSOControl*)a;
    ctrl->runLoop();
}

/**
 *  Use the ADC to sample coupling pin
 *  No need to hurry, it is a single sample done whenever the ADC is free
 *  So analogRead is fine
 */
void          DSOControl::updateCouplingState()
{
    couplingState=couplingFromAdc2();
}

/**
 * 
 * @return 
 */
const char    *DSOControl::geCouplingStateAsText()
{
    switch(couplingState)
    {
        case   DSO_COUPLING_GND: return "GND";break;
        case   DSO_COUPLING_DC: return "DC ";break;
        case   DSO_COUPLING_AC: return "AC ";break;
        default: xAssert(0);break;
    }
    return "???";
}
/**
 * 
 * @return 
 */
DSOControl::DSOCoupling  DSOControl::getCouplingState()
{
    return couplingState;
}

/**
 * 
 */
void DSOControl::runLoop()
{
    xDelay(5);
    int base=millis()&0xffff;
    while(1)
    {
        static int next=(base+TICK) & 0xffff;;
        static int now=(millis()&0xffff);
        static int wait;
        
        base=next&0xffff;        
        if(next<now) next+=0x10000;
        wait=next-now;
        xAssert(wait<=TICK);
        if(wait>0 ) // no wrap
        {
            xAssert(wait<=TICK);         
            xDelay(wait);
        }
        
        PortAMutex.lock(); // make sure we have control over GPIOB
        uint32_t val= GPIOB->regs->IDR;     
        PortAMutex.unlock();
        for(int i=DSO_BUTTON_ROTARY;i<=DSO_BUTTON_OK;i++)
        {
            singleButton &button=_buttons[i];
            if(button.holdOff()) 
                continue;
            
            int k=!(val&(1<<i));
            
            int oldCount=button._pinCounter;
            button.integrate(k);
            button.runMachine(oldCount);
          
        }        
    }
}

/**
 * 
 * @return 
 */
bool DSOControl::setup()
{
#ifdef USE_RXTX_PIN_FOR_ROTARY         
     attachInterrupt(ALT_ROTARY_LEFT,_myInterruptRE,(void *)DSO_BUTTON_UP,CHANGE );
     attachInterrupt(ALT_ROTARY_RIGHT,_myInterruptRE,(void *)DSO_BUTTON_DOWN,CHANGE );
#else
    attachRE(DSO_BUTTON_UP);
    attachRE(DSO_BUTTON_DOWN);
#endif
    xTaskCreate( trampoline, "Control", 250, this, DSO_CONTROL_TASK_PRIORITY, &taskHandle );       
    return true;
}
/**
 * 
 * @param a
 */

void DSOControl::interruptRE(int a)
{   
#ifdef USE_RXTX_PIN_FOR_ROTARY    
  int pinstate= ((( ROTARY_GPIO->regs->IDR))>>ROTATY_SHIFT)&3;
   // Determine new state from the pins and state table.
  state = ttable[state & 0xf][pinstate];
  // Return emit bits, ie the generated event.
  switch(state&DIR_MASK)
  {
    case DIR_CW:
            debugUp++;
            counter++;
            break;
    case DIR_CCW: 
            debugDown++;
            counter--;
            break;
    default: 
            break;
  }
#else
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
#endif  
}
/**
 * 
 * @param button
 * @return 
 */
bool DSOControl::getButtonState(DSOControl::DSOButton button)
{
    return _buttons[button]._pinState;
}
/**
 * 
 * @param button
 * @return 
 */
int  DSOControl::getButtonEvents(DSOButton button)
{
    // Avoid disabling interrupts    
    int evt = __atomic_exchange_n( &(_buttons[button]._events), 0, __ATOMIC_SEQ_CST);
    return evt;
}

/**
 * 
 * @return 
 */
int  DSOControl::getRotaryValue()
{
    // Avoid disabling interrupts    
    int evt = __atomic_exchange_n( &(counter), 0, __ATOMIC_SEQ_CST);
    return evt;
}

/**
 * \fn setInputGain
 * \brief SENSEL control, 2 stage amplifier
 * First stage is SENSEL3 (PA4) : /1 or /120
 * Second stage is SENSEL0..2 (PA1,PA2,PA3= : /1../40
 * 
 * The 2nd stage order is weird,
 * /40 2
 * /20 3
 * /10 0
 * /4  7
 * /2  6
 * /1  4
 * 
 * @param val
 * @return 
 */
int  DSOControl::setInputGain(int val)
{    
    int set=val&0xf; // 4 useful bits
    int unset=(~set)&0x0f;    
    GPIOA->regs->BSRR=(set<<1);
    GPIOA->regs->BRR=(unset<<1);
    return 0;
}

//