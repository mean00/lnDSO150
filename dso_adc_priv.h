// this is 1/Gain for each range
// i.e. attenuation
//

#define ADC_TIMER Timer2
#define ADC_TIMER_CHANNEL TIMER_CH1
#define ADC_TIMER_COUNT   1



class InterruptStats
{
public:
  int       watchDog;
  int       rawWatchdog;
  int       adcEOC;
  int       spurious;
  uint32_t  startAt;
  int       round;
  int       eocTriggered;
  int       eocIgnored;
  
  InterruptStats()
  {
    watchDog=0;
    adcEOC=0;
    startAt=0;
    spurious=0;
    rawWatchdog=0;
    eocIgnored=0;
    eocTriggered=0;

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