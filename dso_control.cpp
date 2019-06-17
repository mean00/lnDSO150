
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

#define TICK                  10 // 10 ms
#define LONG_PRESS_THRESHOLD (2000/TICK) // 1s
#define SHORT_PRESS_THRESHOLD (3)
#define HOLDOFF_THRESHOLD     (100/TICK)
#define COUNT_MAX             3


int debugUp=0;
int debugDown=0;

int ampMapping[16]=
{
    1 , // GND
    12, // x14
    14, // x7
    15, // x3.5
    8, // x1.4
    13, // x0.7
    11, // x0.35
    
    4, // /7
    6, // /14
    7, // /29
    0, // /71
    5, // /143
    3,  // /286
    
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

#define TX_PIN PA9
#define RX_PIN PA10

#define COUPLING_PIN PA5
  
#define SENSEL_PIN PA1 //(1..4)
  
extern xMutex PortAMutex; // lock against LCD  
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
                                if(_pinCounter>=COUNT_MAX) _pinCounter=COUNT_MAX-1;
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
static DSOControl::DSOCoupling couplingFromAdc()
{
    pinMode(COUPLING_PIN,INPUT_ANALOG);
    adc_reg_map *regs=  PIN_MAP[COUPLING_PIN].adc_device->regs; //PIN_MAP[COUPLING_PIN].adc_device.regs;
    uint32_t sqr3=regs->SQR3;
    rawCoupling=analogRead(COUPLING_PIN);
    regs->SQR3=sqr3;
    if(rawCoupling>3500)      
        return DSOControl::DSO_COUPLING_AC;
    if(rawCoupling<500)       
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
    pinMode(TX_PIN,INPUT_PULLUP); // ok
    pinMode(RX_PIN,INPUT_PULLUP);
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
    couplingDevice= PIN_MAP[COUPLING_PIN].adc_device;
    couplingChannel=PIN_MAP[COUPLING_PIN].adc_channel;
    couplingState=couplingFromAdc();
    
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
    couplingState=couplingFromAdc();
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
    while(1)
    {
        xDelay(TICK);
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
     attachInterrupt(TX_PIN,_myInterruptRE,(void *)DSO_BUTTON_UP,CHANGE );
     attachInterrupt(RX_PIN,_myInterruptRE,(void *)DSO_BUTTON_DOWN,CHANGE );
#else
    attachRE(DSO_BUTTON_UP);
    attachRE(DSO_BUTTON_DOWN);
#endif
    xTaskCreate( trampoline, "Control", 250, this, DSO_CONTROL_TASK_PRIORITY, &taskHandle );       
}
/**
 * 
 * @param a
 */

void DSOControl::interruptRE(int a)
{   
#ifdef USE_RXTX_PIN_FOR_ROTARY    
  int pinstate= ((( GPIOA->regs->IDR))>>9)&3;
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
    noInterrupts(); // very short, better than sem ?
    int evt=_buttons[button]._events;
    _buttons[button]._events=0;
    interrupts();
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
    val=ampMapping[val]; // monotone version
    int set=val&0xf; // 4 useful bits
    int unset=(~set)&0x0f;    
    GPIOA->regs->BSRR=(set<<1);
    GPIOA->regs->BRR=(unset<<1);
    return 0;
}

//