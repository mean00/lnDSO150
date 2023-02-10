/*
 *  (C) 2021 MEAN00 fixounet@free.fr
 *  See license file
 * 
 *  
 */

#pragma once
#include "lnDma.h"
#include "lnADC.h"
#include "lnTimer.h"

typedef void (adcCb)(int nb,bool mid,int segment);
struct lnDSOADCCallbacks;
/**
 * 
 * @param instance
 */
class lnAdcTimer;
class lnDSOAdc : public lnBaseAdc
{
public:
    
    enum lnDSOADC_State
    {
        IDLE=0,
        WARMUP=8,
        ARMING=1,
        ARMING2=2,
        ARMED=3,
        TRIGGERED=4
    };
    
                lnDSOAdc(int instance,int timer, int channel);
   bool         setCallbacks(const lnDSOADCCallbacks *cb) {_cb=cb;return true;}
   virtual      ~lnDSOAdc();
   bool         setSource(int fq, lnPin pins,lnADC_DIVIDER divider, lnADC_CYCLES cycles, int overSamplingLog2);
   bool         startDmaTransfer(int n,  uint16_t *output) ;
   bool         startTriggeredDma(int n,  uint16_t *output) ;
   bool         setWatchdog(int low, int high);
   void         stopCapture();
   void         setCb(  adcCb *c) { _captureCb=c;}
   void         endCapture();
public:
    static void dmaDone_(void *foo, lnDMA::DmaInterruptType typ);
    static void dmaTriggerDone_(void *foo, lnDMA::DmaInterruptType typ);
    void        delayIrq();
protected:   
    void        dmaDone();
    void        dmaTriggerDone(lnDMA::DmaInterruptType typ);
    void        armTimer();
    lnDSOADC_State _state;
    int         _timer,_channel,_fq;
    lnDMA       _dma;
    lnAdcTimer  *_adcTimer;
    adcCb       *_captureCb;
    lnPin       _pin;
    int         _nbSamples;
    uint16_t   *_output;
    uint32_t    _triggerLocation;
    lnDelayTimer _delayTimer;
    const lnDSOADCCallbacks *_cb;
    
};

typedef   lnDSOAdc::lnDSOADC_State (LN_GetWatchdogCb)(lnDSOAdc::lnDSOADC_State state, int &mn, int &mxv);
typedef   bool (LN_Lookup)(lnDSOAdc::lnDSOADC_State state,uint16_t *data,int size,int &index);
typedef   int  (LN_GetDelayUs)(int currentLocation, int triggerLocation, int loopSize);
/**
 */
struct lnDSOADCCallbacks
{
    LN_GetWatchdogCb *getWatchdog;
    LN_Lookup        *lookup;
    LN_GetDelayUs    *getDelayUs;
};


// EOF

