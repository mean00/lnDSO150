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
    _tex=tex;
}
/**
 */
void DSO_portArbitrer::setInputDirectionValue(uint32_t v)
{
    _inputDirection=v;
}
/**
 */
void DSO_portArbitrer::beginInput()
{
    _tex->lock();
    _oldDirection=*_directionPort;
    *_directionPort=_inputDirection;
}
/**
 */
void DSO_portArbitrer::endInput()
{
    *_directionPort=_oldDirection;
    _tex->unlock();
}
/**
 */
void DSO_portArbitrer::beginLCD()
{
    _tex->lock();    
}
/**
 */
void DSO_portArbitrer::endLCD()
{
    _tex->unlock();
}
// EOF


