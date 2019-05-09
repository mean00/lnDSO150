/**
    Global includes & some global vars
 
 */
#pragma once

#include <Wire.h>
#include "SPI.h"
#include "MapleFreeRTOS1000.h"
#include "MapleFreeRTOS1000_pp.h"
#include "dso_adc.h"
#include "dso_calibrate.h"
#include "Adafruit_GFX.h"
#include "Adafruit_TFTLCD_8bit_STM32.h"
#include "dso_control.h"
#include "dso_capture.h"


#define analogInPin  PA0
#define triggerPin   PA8
#define vRefPin      PB8 // Trigger reference voltage

#define DSO_WAVEFORM_HEIGHT 192
#define DSO_WAVEFORM_WIDTH  240
#define DSO_WAVEFORM_OFFSET ((240-DSO_WAVEFORM_HEIGHT-2)/2)

#define CURRENT_HASH 0x1235
extern uint16_t calibrationHash;
extern uint16_t calibrationDC[16];
extern uint16_t calibrationAC[16];


extern Adafruit_TFTLCD_8bit_STM32 *tft;
extern DSOControl                 *controlButtons;
extern DSOCapture                 *capture;



// EOF
