DSOSTM32Duino

This is a new firmware for the DSO 150/DSO shell cheap oscilloscope.
It is using the framework provided by the  (very nice) STM32duino project, R Clarck version.
Please note that it is using cmake-arduino-stm32 as a build system.

Features :

* Screen support for both  ST7789 and ILI9341, autodetected
* Supports vanilla hardware and usb+enhanced rotary encoder mod through compiliation switch
* Zero level and fine voltage calibration
* Smart use of ADC & gain stages to provide maximum accuracy
* Trigger : Rising, Falling, both
* Auto setup : Press "OK" for 3 seconds 
* Settable test signal. Press the rotary encoder for 3 sec to enter the menu.
* Single shot or repeat mode
* USB support  (not really used as of today)
* Multithreaded so that it should be relatively responsive
* Using ADC in DMA or Timer mode  depending on the time scale


![screenshot](gfx/front.jpg?raw=true "front")


The code is using parts derived from pig-o-scope and other projects for the ADC management
Open DSO150 was used as documentation for the circuit.
