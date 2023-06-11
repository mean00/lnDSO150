
#pragma once
#include "lnArduino.h"
/**
 DSO_portArbitrer*/
class DSO_portArbitrer
{
  public:
    DSO_portArbitrer(int port, xMutex *tex);
    void setInputDirectionValue(uint32_t v, uint32_t v2);
    void setInputValue(uint32_t v);
    void beginInput();
    void endInput();
    void beginLCD();
    void endLCD();
    uint32_t currentDirection(int offset)
    {
        return _directionPort[offset];
    }
    uint32_t currentValue()
    {
        return _valuePort[0];
    }
    void setValue(uint32_t v)
    {
        _valuePort[0] = v;
    }
    void setDirection(int offset, uint32_t val)
    {
        _directionPort[offset] = val;
    }

  protected:
    xMutex *_tex;
    volatile uint32_t *_directionPort;
    uint32_t _oldDirection, _oldDirection2;
    uint32_t _inputDirection, _inputDirection2;
    volatile uint32_t *_valuePort;
    uint32_t _intputValue, _oldInput;

    uint32_t _oldTime, _maxLockOut;
};