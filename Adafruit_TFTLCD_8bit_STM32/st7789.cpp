// IMPORTANT: LIBRARY MUST BE SPECIFICALLY CONFIGURED FOR EITHER TFT SHIELD
// OR BREAKOUT BOARD USAGE.  SEE RELEVANT COMMENTS IN Adafruit_TFTLCD.h

// Graphics library by ladyada/adafruit with init code from Rossum
// MIT license

/*

Graphics-test Benchmark  Time (microseconds)
--------------------------------------------
Screen fill              62999
Text                     20483
Lines                    173830
Horiz/Vert Lines         7604
Rectangles (outline)     5958
Rectangles (filled)      148717
Circles (filled)         91783
Circles (outline)        75096
Triangles (outline)      55138
Triangles (filled)       90553
Rounded rects (outline)  27257
Rounded rects (filled)   186639
*/

#include "Adafruit_TFTLCD_8bit_STM32.h"
#include "Adafruit_TFTLCD_8bit_STM32_priv.h"
#include "st7789.h"

/**
 * 
 * @param x
 */
void Adafruit_TFTLCD_8bit_STM32_ST7789::setRotation(uint8_t x)
{
    // Call parent rotation func first -- sets up rotation flags, etc.
    Adafruit_GFX::setRotation(x);
    uint16_t t;
    switch (rotation) 
    {
    case 1:
      t =  ILI9341_MADCTL_MV | ILI9341_MADCTL_MY;
      break;
    case 2:
      t = 0 ;
      break;
    case 3:
      t = ILI9341_MADCTL_MX | ILI9341_MADCTL_MV;      
      break;
    case 0:
    default:
      t =  ILI9341_MADCTL_MX | ILI9341_MADCTL_MY;
     break;
   }
   t|=ILI9341_MADCTL_RGB;
   writeRegister8(ILI9341_MADCTL, t ); // MADCTL
  
}