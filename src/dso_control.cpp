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
#include "lnADC.h"
#include "gd32/nvm_gd32.h"
#include "DSO_nvmId.h"

extern lnNvm                    *nvm;

#define TICK                  10 // 10 ms
#define LONG_PRESS_THRESHOLD (1000/TICK) // 1s
#define SHORT_PRESS_THRESHOLD (3) // 20 ms
#define HOLDOFF_THRESHOLD     (100/TICK)
#define COUNT_MAX             4

extern uint16_t directADC2Read(int pin);

extern DSO_portArbitrer *arbitrer;
lnSimpleADC *couplingAdc=NULL;

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


#define ButtonToPin(x)    (PB0+(x))
//#define pinAsInput(x)     lnPinMode(ButtonToPin(x),lnINPUT_FLOATING);

#define attachRE(x)       lnExtiAttachInterrupt(ButtonToPin(x),LN_EDGE_FALLING,_myInterruptRE,(void *)x );

#define NB_BUTTONS 8
/**
 * 
 */
static void pinAsInput(int x)
{
     if(x & DSO_CONTROL_BUTTON_PORT_A)
     {
        lnPinMode(PA0+(x&(DSO_CONTROL_BUTTON_PORT_A-1)),lnINPUT_PULLUP);
     }else
     {
        lnPinMode(ButtonToPin(x),lnINPUT_PULLUP);
     }
}
  

static int rawCoupling;  
/**
 */
#include "dso_singleButton.h"

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
 * @return 
 */
uint32_t DSOControl::snapshot()
{
uint32_t val;
    arbitrer->beginInput();   
    val= lnReadPort(1); // read all bits from portB        
    val=0xffff^val;     // active low, so invert it
    arbitrer->endInput();        
    return val;
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
/**
 * 
 * @return 
 */

DSOControl::DSOCoupling couplingFromAdc2()
{
#ifdef USE_FNIRSI_BUTTON
    int val=lnDigitalRead(COUPLING_PIN)+lnDigitalRead(KEY_PIN)*2;
    switch(val)
    {
        default:
        case 0: return  DSOControl::DSO_COUPLING_GND;break;
        case 2: return  DSOControl::DSO_COUPLING_DC;break;//dc
        case 1: return  DSOControl::DSO_COUPLING_AC;break;//dc
    }
    return  DSOControl::DSO_COUPLING_GND;
#else
    couplingAdc->setPin(COUPLING_PIN);   // Reset ADC1    
    int rawCoupling=couplingAdc->simpleRead();
    //Logger("C:%d\n",rawCoupling);
    if(rawCoupling>3200)      
        return DSOControl::DSO_COUPLING_AC;
    if(rawCoupling<1000)       
        return DSOControl::DSO_COUPLING_GND;
    return DSOControl::DSO_COUPLING_DC;
#endif
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
    _inverted=false;
    
#ifdef USE_RXTX_PIN_FOR_ROTARY    
    #define PREPARE_PIN(x)  lnPinMode(x,lnOUTPUT);  digitalWrite(x,1);lnPinMode(x,lnINPUT_PULLUP);     
    PREPARE_PIN(ALT_ROTARY_LEFT)
    PREPARE_PIN(ALT_ROTARY_RIGHT)
#endif            

            
    for(int i=0;i<4;i++)    
        lnPinMode(SENSEL_PIN+i,lnOUTPUT); // SENSEL            
            
    uint32_t oldDir=arbitrer->currentDirection(0), oldDir2=arbitrer->currentDirection(1);
    uint32_t oldVal=arbitrer->currentValue();        
    
#ifndef USE_RXTX_PIN_FOR_ROTARY                
    pinAsInput(ButtonMapping[DSO_BUTTON_UP]);
    pinAsInput(ButtonMapping[DSO_BUTTON_DOWN]);
#endif
    
    pinAsInput(ButtonMapping[DSO_BUTTON_ROTARY]);
    pinAsInput(ButtonMapping[DSO_BUTTON_VOLTAGE]);
    pinAsInput(ButtonMapping[DSO_BUTTON_TIME]);
    pinAsInput(ButtonMapping[DSO_BUTTON_TRIGGER]);
    pinAsInput(ButtonMapping[DSO_BUTTON_OK]);
    
   
    
    // Ok now the direction is correct, memorize it
    arbitrer->setInputDirectionValue(arbitrer->currentDirection(0),arbitrer->currentDirection(1));    
    arbitrer->setInputValue(arbitrer->currentValue());
    // now restore everything
    arbitrer->setDirection(0,oldDir); // restore...
    arbitrer->setDirection(1,oldDir2); // restore...
    arbitrer->setValue(oldVal); // restore...
    // 
    // Use ADC1 to scan the coupling pin independantly
    //
#ifdef USE_FNIRSI_BUTTON
    lnPinMode(COUPLING_PIN,lnINPUT_PULLUP);
    lnPinMode(KEY_PIN,lnINPUT_PULLUP);
#else    
    lnPinMode(COUPLING_PIN,lnADC_MODE);
    couplingAdc=new  lnSimpleADC(1, COUPLING_PIN);
    couplingState=couplingFromAdc2();    
#endif
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
    // clear events when switching callback
    for(int button=DSO_BUTTON_UP;button<= DSO_BUTTON_OK;button++)
    {
        _buttons[button].reset();
    }
    counter=0;
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
        
        // check coupling...
        DSOControl::DSOCoupling newCoupling=couplingFromAdc2();
        bool couplingChanged=false;
        if(newCoupling!=couplingState) 
        {
            couplingChanged=true;
            couplingState=newCoupling;
        }
        if(couplingChanged)   
        {
            if(_cb)
                _cb(DSOControl::DSOEventCoupling);
        }

        uint32_t val=snapshot();
        
        
        int changed=0;
        for(int i=DSO_BUTTON_ROTARY;i<=DSO_BUTTON_OK;i++)
        {
            singleButton &button=_buttons[i];
            if(button.holdOff()) 
                continue;
            
            int shift=ButtonMapping[i];
            int mask;
            if(shift & DSO_CONTROL_BUTTON_PORT_A)
                mask=!lnDigitalRead(PA0+(shift &(DSO_CONTROL_BUTTON_PORT_A-1)));
            else
                mask=1<<ButtonMapping[i];
            
            int k=(val&mask);
            
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
  int inc=1;
  if(_inverted) inc=-1;
  switch(state&DIR_MASK)
  {
    case DIR_CW:
            debugUp++;
            counter+=inc;
            break;
    case DIR_CCW: 
            debugDown++;
            counter-=inc;
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
 */
void DSOControl::purgeEvent()
{
    for(int i=DSO_BUTTON_UP;i<=DSO_BUTTON_OK;i++)
    {
        _buttons[i]._events=0;
    }
}
/**
 * 
 * @return 
 */
int DSOControl::getQButtonEvent()
{
    for(int i=DSO_BUTTON_UP;i<=DSO_BUTTON_OK;i++)
    {
        int evt=_buttons[i]._events;
        if(evt)
        {
            _buttons[i]._events=0; // race is possible here, but we dont care
            return (evt<<16)+i;
        }
    }
    return 0;
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
void  DSOControl::setInputGain(int val)
{    
    int set=(val)&0xf; // 4 useful bits
    int unset=(~set)&0xf;    
    volatile uint32_t *portA=lnGetGpioToggleRegister(0);
    uint32_t v=set+(unset<<16);
    *portA=v<<1;
}
/**
 * 
 */
void          DSOControl::loadSettings()
{
  uint8_t value;
  if(!nvm->read(NVM_INVERT_ROTARY,1,(uint8_t *)&value))
  {
      Logger("Cannot read invert setting\n");
      return;  
  }
  _inverted=value;
}
/**
 * 
 */
void          DSOControl::saveSettings()
{
  uint8_t value=_inverted;
  if(!nvm->write(NVM_INVERT_ROTARY,1,(uint8_t *)&value))
  {
      Logger("Cannot write invert setting\n");
  }  
}


//
