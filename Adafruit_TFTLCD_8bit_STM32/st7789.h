

#pragma once
#include "ili9341.h"

/**
 * The ST7789 might be  not identified the right way
 * The only noticeable change compared to the ILI9341 is the screen rotation
 * It might due to the physical screen, and not the controller itself
 */
class Adafruit_TFTLCD_8bit_STM32_ST7789 : public  Adafruit_TFTLCD_8bit_STM32_ILI9341
{
public:    
    virtual void     setRotation(uint8_t x);
};


// EOF
