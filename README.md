DSOSTM32Duino

This is a very work-in-progress project to write a firmware for the DSO shell, using STM32duino as a basis.

It uses my port of arduino-stm32-cmake as a build system

It is very early, see the wiki for build instruction (linux only)


Status so far:
* Screen ok (with both ST7789 and ILI9341)
* Support vanilla hardware and usb+enhanced rotary encoder mod through compiliation switch
* Support voltage change/ time change / offset change / trigger
* Micro USB port used a serial 
* Calibration ok

What does NOT work :
* Trigger in dma mode (with speed faster than 5ms / div)
* Rising trigger in time mode (speed slower than 5ms)
* Tons of bugs

![screenshot](gfx/front.jpg?raw=true "front")
