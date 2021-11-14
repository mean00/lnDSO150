/*
 *  (C) 2021 MEAN00 fixounet@free.fr
 *  See license file
 * 
 *  
 */

#pragma once
#include "lnDma.h"
#include "lnADC.h"

typedef void (adcCb)(int nb,bool mid);

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
        ARMING=1,
        ARMING2=2,
        ARMED=3,
        TRIGGERED=4
    };
    
                lnDSOAdc(int instance,int timer, int channel);
   virtual      ~lnDSOAdc();
   bool         setSource(int fq, lnPin pins,lnADC_DIVIDER divider, lnADC_CYCLES cycles, int overSamplingLog2);
   bool         startDmaTransfer(int n,  uint16_t *output) ;
   bool         startTriggeredDma(int n,  uint16_t *output) ;
   bool         setWatchdog(int low, int high);
   void         stopCapture();
   void         setCb(adcCb *c) { _cb=c;}
   void         endCapture();
public:
    static void dmaDone_(void *foo, lnDMA::DmaInterruptType typ);
    static void dmaTriggerDone_(void *foo, lnDMA::DmaInterruptType typ);
    void        irqHandler(void);
protected:   
    void        dmaDone();
    void        dmaTriggerDone(lnDMA::DmaInterruptType typ);
    lnDSOADC_State _state;
    int         _timer,_channel,_fq;
    lnDMA       _dma;
    xBinarySemaphore _dmaSem;
    lnAdcTimer *_adcTimer;
    adcCb      *_cb;
    int         _nbSamples;
    int         _pin;
    
    int         _dmaLoop;    
    uint16_t   *_output;
    int         _nb;
    uint32_t    _triggerLocation;
    
};
// EOF

