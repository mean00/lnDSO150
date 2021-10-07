
#pragma once
#include "lnArduino.h"
/**
 DSO_portArbitrer*/
class DSO_portArbitrer
{
public:
                     DSO_portArbitrer(int port,  xMutex *tex);
                void setInputDirectionValue(uint32_t v);
                void beginInput();
                void endInput();
                void beginLCD();
                void endLCD();
                uint32_t currentDirection() { return *_directionPort;}
protected:
    xMutex      *_tex;
    volatile uint32_t    *_directionPort;
    uint32_t    _oldDirection;
    uint32_t    _inputDirection;
};