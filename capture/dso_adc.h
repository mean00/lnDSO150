/*
 *  (C) 2021 MEAN00 fixounet@free.fr
 *  See license file
 * 
 *  
 */

#pragma once
#include "lnDma.h"
#include "lnADC.h"

typedef void (adcCb)(int nb);

/**
 * 
 * @param instance
 */
class lnAdcTimer;
class lnDSOAdc : public lnBaseAdc
{
public:
                lnDSOAdc(int instance,int timer, int channel);
   virtual      ~lnDSOAdc();
   bool         setSource(int fq, lnPin pins,lnADC_DIVIDER divider, lnADC_CYCLES cycles, int overSamplingLog2);
   bool         startDmaTransfer(int n,  uint16_t *output) ;
   void         stopCapture();
   void         setCb(adcCb *c) { _cb=c;}
   void         endCapture();
public:
    static void dmaDone_(void *foo, lnDMA::DmaInterruptType typ);
protected:   
    void        dmaDone();
    int         _timer,_channel,_fq;
    lnDMA       _dma;
    xBinarySemaphore _dmaSem;
    lnAdcTimer *_adcTimer;
    adcCb      *_cb;
    int         _nbSamples;
};
// EOF
