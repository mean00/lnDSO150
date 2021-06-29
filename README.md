DSOSTM32Duino
=============

This is a new firmware for the DSO 150/DSO shell cheap oscilloscope.
It is using the framework provided by the  (very nice) STM32duino project, R Clark version.
Please note that it is using cmake-arduino-stm32 as a build system.

![screenshot](wiki/yt.png?raw=true "front")
[Small Youtube demo ](https://youtu.be/3X-XcUKmUwo "Youtube")

__Features__ :

* Screen support for both  ST7789 and ILI9341, autodetected
* Support both STM32F103 and GD32F303 (the latter is ~ 2 times faster)
* Supports vanilla hardware and usb+enhanced rotary encoder mod through compilation switch
* Zero level and fine voltage calibration
* Smart use of ADC & gain stages to provide maximum accuracy
* Trigger : Rising, Falling, both
* Auto setup : Press "OK" for 3 seconds 
* Settable test signal. Press the rotary encoder for 3 sec to enter the menu.
* Single shot or repeat mode
* USB support : for version >= 1.0.7 you can control / retrieve data over usb (python samples scripts in the source tree)
* Multithreaded so that it should be relatively responsive
* Using ADC in  ADC clock or Timer mode  depending on the time scale
* Frequency down to 5us / division using dual ADC capture mode

**NB:** Recent versions are using PB14 and PB15 for the rotary encoder mod!
That mod **REALLY** makes the overall experience better.


![screenshot](gfx/front.jpg?raw=true "front")


The code is using parts derived from pig-o-scope and other projects for the ADC management
Open DSO150 was used as documentation for the circuit.
