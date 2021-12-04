/**
 The portB is used at the same time by the LCD and by the key inputs
 The arbitrer is here to make sure they do not conflict by locking and configuring
 the portB exclusively between the 2 use cases
 
 The default is LCD, input is preempting it
 Care must be taken to drop the LCD from time to time (~ 10 ms) so we dont miss 
 key input
 
 
 */
#include "DSO_portBArbitrer.h"
/**
 */
DSO_portArbitrer::DSO_portArbitrer(int port,  xMutex *tex)
{
    _directionPort=lnGetGpioDirectionRegister(port);
    _valuePort=lnGetGpioValueRegister(port);
    _tex=tex;
    _maxLockOut=0;
}
/**
 */
void DSO_portArbitrer::setInputDirectionValue(uint32_t v, uint32_t v2)
{
    _inputDirection=v;
    _inputDirection2=v2;
}
/**
 * 
 * @param v
 */
void DSO_portArbitrer::setInputValue(uint32_t v)
{
    _intputValue=v;
}

#define WAIT_A_BIT asm("nop");asm("nop");asm("nop");
/**
 */
void DSO_portArbitrer::beginInput()
{
    _tex->lock();
    WAIT_A_BIT
    _oldDirection=_directionPort[0];
    _oldDirection2=_directionPort[1];
    _oldInput=*_valuePort;
    _directionPort[0]=_inputDirection;
    _directionPort[1]=_inputDirection2;
    *_valuePort=_intputValue;
    WAIT_A_BIT
}
/**
 */
void DSO_portArbitrer::endInput()
{
    _directionPort[0]=_oldDirection;
    _directionPort[1]=_oldDirection2;
    *_valuePort=_oldInput;
    WAIT_A_BIT
    _tex->unlock();
}
#if 1
extern
#endif
bool lcdHasArbiter;
/**
 */
void DSO_portArbitrer::beginLCD()
{
    _tex->lock(); 
    lcdHasArbiter=true;
    _oldTime=lnGetUs();
}
/**
 */
void DSO_portArbitrer::endLCD()
{
    uint32_t nw=lnGetUs();
    lcdHasArbiter=false;
    _tex->unlock();
    nw-=_oldTime;
    if(nw<50000)
    {
        if(nw>_maxLockOut) _maxLockOut=nw;
    }
}
// EOF


