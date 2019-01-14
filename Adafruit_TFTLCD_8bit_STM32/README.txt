This is a library for the 8 bit parallel driven TFT displays, ported from the Adafruit library [https://github.com/adafruit/TFTLCD-Library] and adapted to STM32F103Cx controller. No other CPU architecture is supportted at the moment.

The scope of this library is to achieve highest display update rate while maintaing the compatibility with original Adafruit display API.
Only tested with ILI9328 and "unknown" ILI9341-compatitble driver chips.

Contribution from users for other display control types is welcome and made easy by allocating separate files for each controller type in part. 

How to use:
- Place the Adafruit_TFT library folder your <arduinosketchfolder>/libraries/ folder. You may need to create the libraries subfolder if its your first library. Restart the IDE.

- Also requires the Adafruit_GFX library for Arduino. https://github.com/adafruit/Adafruit-GFX-Library . Alternatively, you can change Adafruit_TFTLCD_8bit_STM32.h to #include <Adafruit_GFX_AS.h>, instead.

- Pin connections can be configured near the top of Adafruit_TFTLCD_8bit_STM32.h . Note that the data lines have to be on consequtive pins of the same output register (PB0...PB7, by default).
  To use the higher pins (PX8..PX15), set TFT_DATA_SHIFT to 8. The four control pins must all be one output register (the same as the data pins, or a different one), but can be freely
  assigned within the register. The TFT reset line can be assigned, freely.
