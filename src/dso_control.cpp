/*
 * In alt mode the rotary encoder is using interrupt
 * Other buttons are polled
 * 
 * 
 Rotary encoder part : Derived from  Rotary encoder handler for arduino * Copyright 2011 Ben Buxton. Licenced under the GNU GPL Version 3. * Contact: bb@cactii.net
 Debounce part derived from http://www.kennethkuhn.com/electronics/debounce.c
 * 
 */

#include "lnArduino.h"
#include "dso_control.h"
#include "dso_control_internal.h"
#include "pinConfiguration.h"
#include "dso_config.h"
#include "DSO_portBArbitrer.h"

#define TICK                  10 // 10 ms
#define LONG_PRESS_THRESHOLD (1000/TICK) // 1s
#define SHORT_PRESS_THRESHOLD (3) // 20 ms
#define HOLDOFF_THRESHOLD     (100/TICK)
#define COUNT_MAX             4

extern uint16_t directADC2Read(int pin);

extern DSO_portArbitrer *arbitrer;

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
#define pinAsInput(x)     lnPinMode(ButtonToPin(x),lnINPUT_PULLUP);
#define attachRE(x)       lnExtiAttachInterrupt(ButtonToPin(x),LN_EDGE_FALLING,_myInterruptRE,(void *)x );

#define NB_BUTTONS 8

  

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

    int runMachine(int oldCount)
    {

        int oldPin=_pinState;
        int newPin=_pinCounter>(COUNT_MAX-1);
        int r=0;
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
                        r++;
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
                    r++;
                }
                break;
        }                       
        _pinState=newPin;
        return r;
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
extern void useAdc2(bool use);
int ints=0;
/**
 * \brief This one is for left/right
 * @param a
 */
static void _myInterruptRE(lnPin pin,void *a)
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
extern void Logger(const char *fmt...);
static DSOControl::DSOCoupling couplingFromAdc2()
{
    
    useAdc2(true);
    lnPinMode(COUPLING_PIN,lnADC_MODE);
    rawCoupling= directADC2Read(COUPLING_PIN);    
    
    //Logger("Coupling=%d\n",rawCoupling);
    
    useAdc2(false);
    if(rawCoupling>3200)      
        return DSOControl::DSO_COUPLING_AC;
    if(rawCoupling<1000)       
        return DSOControl::DSO_COUPLING_GND;
    return DSOControl::DSO_COUPLING_DC;
}
/**
 * 
 */
DSOControl::DSOControl(ControlEventCb *c)
{
    state = R_START;
    instance=this;
    counter=0;
    _cb=c;
    
#ifdef USE_RXTX_PIN_FOR_ROTARY    
    #define PREPARE_PIN(x)  lnPinMode(x,lnOUTPUT);  digitalWrite(x,1);lnPinMode(x,lnINPUT_PULLUP);     
    PREPARE_PIN(ALT_ROTARY_LEFT)
    PREPARE_PIN(ALT_ROTARY_RIGHT)
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
        lnPinMode(SENSEL_PIN+i,lnOUTPUT); // SENSEL
    
    lnPinMode(COUPLING_PIN,lnADC_MODE);
    couplingState=couplingFromAdc2();    
    // Ok now the direction is correct, memorize it
    arbitrer->setInputDirectionValue(arbitrer->currentDirection());
    
}
/**
 * 
 * @param newCb
 * @return 
 */
bool DSOControl::changeCb(ControlEventCb *newCb)
{
    noInterrupts();
    _cb=newCb;
    interrupts();
    return true;
}
/**
 * 
 * @param a
 */
static void trampoline(void *a)
{
    DSOControl *ctrl=(DSOControl*)a;
    ctrl->runLoop();
}

/**
 * Read the coupling pin every 300 ms or so
 * It's value depends on the coupling selector
 * ~ 0 / ~ 2000 / ~ 4000
 */
static uint32_t lastUpdate=0;
void          DSOControl::updateCouplingState()
{
    uint32_t now=millis();
    if(now>lastUpdate+500)
    {
        couplingState=couplingFromAdc2();
        
        lastUpdate=now;
    }
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
#ifdef USE_RXTX_PIN_FOR_ROTARY    
    lnExtiEnableInterrupt(ALT_ROTARY_LEFT); lnExtiEnableInterrupt(ALT_ROTARY_RIGHT);
#endif    
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
        arbitrer->beginInput();        
        uint32_t val= lnReadPort(1); // read all bits from portB        
        arbitrer->endInput();
        
        
        int changed=0;
        for(int i=DSO_BUTTON_ROTARY;i<=DSO_BUTTON_OK;i++)
        {
            singleButton &button=_buttons[i];
            if(button.holdOff()) 
                continue;
            
            int k=!(val&(1<<i));
            
            int oldCount=button._pinCounter;
            button.integrate(k);
            changed+=button.runMachine(oldCount);          
        }    
        if(counter)
            changed++;
        if(changed) 
        {
            if(_cb)
            {
                _cb(DSOControl::DSOEventControl);
            }
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
#define ROT_EDGE LN_EDGE_BOTH //LN_EDGE_FALLING //LN_EDGE_BOTH
     lnExtiAttachInterrupt(ALT_ROTARY_LEFT, ROT_EDGE,_myInterruptRE,(void *)DSO_BUTTON_UP);
     lnExtiAttachInterrupt(ALT_ROTARY_RIGHT,ROT_EDGE,_myInterruptRE,(void *)DSO_BUTTON_DOWN);
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
  int pinstate= (lnReadPort(ROTARY_GPIO) >>ROTATY_SHIFT)&3;
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
  int pinstate =  (  lnReadPort(ROTARY_GPIO))&3;
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
    noInterrupts();
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
    noInterrupts();
    int evt=counter;
    counter=0;
    interrupts();
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
    volatile uint32_t *portA=lnGetGpioToggleRegister(0);
    *portA=set+(unset<<16);
    return 0;
}
//