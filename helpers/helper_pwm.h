
#pragma once

bool setPWMPinFrequency(int pin, int frequency);
bool setTimerFrequency(HardwareTimer *timer, int channel, int periodOnPs, int periodOffPs); // periods in ps
bool setTimerFrequency(HardwareTimer *timer, int channel, int frequency); // in hz
// EOF