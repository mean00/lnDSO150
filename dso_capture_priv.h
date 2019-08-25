/***************************************************
 STM32 duino based firmware for DSO SHELL/150
 *  * GPL v2
 * (c) mean 2019 fixounet@free.fr
 ****************************************************/
#pragma once
#include "fancyLock.h"




/**
 */
typedef struct 
{
    void                        (*stopCapture)(void);    
    DSOCapture::DSO_TIME_BASE   (*getTimeBase)(void);
    bool                        (*prepareSampling)(void);
    const char *                (*getTimeBaseAsText)();
    bool                        (*startCapture) (int count);
    bool                        (*tasklet)();
    bool                        (*nextCapture)(int count);
}CaptureFunctionTable;

/*
 * These helps when dealing with "slow" mode, i.e. when capture is controlled
 * by a timer interrupt
 */
typedef struct TimerTimeBase
{
  DSOCapture::DSO_TIME_BASE timeBase;
  const char    *name;
  int           fq;  
};

extern const TimerTimeBase  timerBases[];
extern const TimeSettings   tSettings[6];
extern VoltageSettings      vSettings[NB_DSO_VOLTAGE];

/**
 */
class DSOCapturePriv : public  DSOCapture
{
public:
    enum TaskletMode
      {
        Tasklet_Idle=0,
        Tasklet_Running=1,
        Tasklet_Parking=2,
      };  
public:
    static const char *getTimeBaseAsTextDma();
    static const char *getTimeBaseAsTextTimer();
    static bool        taskletDma();
    static bool        taskletDmaRunning();
    static bool        taskletDmaCommon(const bool trigger);
    
    static bool        taskletTimer();
    static void        task(void *);
    static bool        startCaptureDma (int count);
    static bool        startCaptureDmaTrigger (int count);
    static bool        startCaptureTimer (int count);
    static bool        startCaptureTimerTrigger (int count);
    static DSO_TIME_BASE getTimeBaseDma();
    static DSO_TIME_BASE getTimeBaseTimer();
    static bool        prepareSamplingDma ();
    static bool        prepareSamplingTimer ();
    static int         voltToADCValue(float v);
    static int         computeFrequency(int samples,uint16_t *data);
    static void        stopCaptureDma();
    static void        stopCaptureTimer();
    static bool        refineCapture(FullSampleSet &set);
    static bool        prepareSampling ();    
    static int         triggeredCapture(int count,float *voltage,CaptureStats &stats);        
    
    static bool        nextCaptureDma(int count);
    static bool        nextCaptureDmaTrigger(int count);
    static bool        nextCaptureTimer(int count);
    static bool        nextCaptureTimerTrigger(int count);
    static bool        nextCapture(void);
    static bool        startCapture (int count);    
    static void        InternalStopCapture();
    static bool        getSamples(CapturedSet **set,int timeoutMs);
protected:
    
public:
    
    static int      currentTimeBase;
    static int      currentVoltageRange;
    static int      lastRequested;
    static int      lastAskedSampleCount;
    static int      triggerValueADC;
    static float    triggerValueFloat;
    static float     voltageOffset;
    static TaskletMode taskletMode;
    static CapturedSet captureSet[2];
    
};
/**
 */
extern FancySemaphore *captureSemaphore;
extern DSOADC   *adc;
extern const CaptureFunctionTable *currentTable;

// EOF