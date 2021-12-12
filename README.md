lnDSO150
========

This is a new firmware for the DSO 150/DSO shell cheap oscilloscope.
It is a follow up of the other firmware (DSO_STM32Duino) with the following changes :
* Simpler codebase, better architecture
* Rewrite using lnArduino instead of stm32duino
* Support for GD32F1,GD32F3,GD32VF1 and STM32F1

![screenshot](assets/web/front.jpg?raw=true "front")


__Features__ :

* Screen support for both  ST7789 and ILI9341, autodetected
* Supports STM32F103 GD32F303, GD32VF103 (the GD32F303 is ~ 2 times faster). You need 128kB of flash minimum.
* Trigger up/down
* Only supports modified unit where rotary encoder is connected to PB14/PB15, see wiki. The rotary encoder will NOT work with unmodified DSO




