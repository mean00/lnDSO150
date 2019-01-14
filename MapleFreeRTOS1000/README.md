This is an quick & dirty update of the FreeRTOS that is bundled with RGClark STM32duino port.

If you use it with arduino-cmake-stm32, dont forget to add 
SET(MapleFreeRTOS1000_RECURSE True)

To your CMakeLists.txt


It also contains a very simple c++ wrapper on top of FreeRTOS to make
the code more readblae

NB, The logging/printf FreeRTOS functions are not mapped
