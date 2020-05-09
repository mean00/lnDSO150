// this is 1/Gain for each range
// i.e. attenuation
//
#pragma once

#define ADC_TIMER Timer2
#define ADC_TIMER_CHANNEL TIMER_CH1
#define ADC_TIMER_COUNT   1
#ifndef ADC_CR1_FASTINT
    #define ADC_CR1_FASTINT (7<<16)
#endif ADC_CR1_FASTINT
#ifndef ADC_CR1_SLOWINT
    #define ADC_CR1_SLOWINT (8<<16)
#endif ADC_CR1_FASTINT
#define ADC_CR1_DUALMASK (0xf<<16)

#include "fancyLock.h"

class InterruptStats
{
public:
  int       watchDog;
  int       rawWatchdog;
  int       adcEOC;
  int       spurious;
  uint32_t  startAt;
  uint32_t  triggeredAt;
  uint32_t  lastEocAt;
  int       round;
  int       eocTriggered;
  int       eocIgnored;
  int       nbCaptured;
  int       nbConsumed;
  int       invalidCapture;
  
  
  InterruptStats()
  {
    watchDog=0;
    adcEOC=0;
    startAt=0;
    spurious=0;
    rawWatchdog=0;
    eocIgnored=0;
    eocTriggered=0;
    triggeredAt=0;
    lastEocAt=0;
    nbCaptured=0;
    nbConsumed=0;
    invalidCapture=0;

  }
  void start()
  {
    watchDog=0;
    rawWatchdog=0;
    adcEOC=0;
    eocIgnored=0;
    eocTriggered=0;
    startAt=millis();
    round++;
  }
  
};

extern InterruptStats adcInterruptStats;
extern int requestedSamples;
extern DSOADC             *instance;

// time based

extern int                  currentIndex;

#define DMA_OVERSAMPLING_COUNT 8 // 8*21us*24 = 4ms
extern uint16_t dmaOverSampleBuffer[DMA_OVERSAMPLING_COUNT] __attribute__ ((aligned (8)));;

enum CaptureState
{
    Capture_idle,
    Capture_armed,
    Capture_dmaDone,
    Capture_timerDone,
    Capture_complete
};
extern CaptureState captureState;
extern int nbTimer;

// util
#define SetCR1(x) {lastCR1=ADC1->regs->CR1=(x);}
extern volatile uint32_t lastCR1;
extern volatile uint32_t cr2;
extern uint32_t vcc;
extern adc_reg_map *adc_Register;
extern FancySemaphore      *dmaSemaphore;

extern void  SPURIOUS_INTERRUPT();
extern voidFuncPtr adcIrqHandler;