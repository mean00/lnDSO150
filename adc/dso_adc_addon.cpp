


#include "dso_adc.h"
#include "dso_adc_priv.h"
#include "fancyLock.h"
#include "dma.h"
#include "adc.h"
void DSOADC::DMA1_CH1_Event() 
{
    SampleSet one,two;

    two.samples=0;
    one.samples=requestedSamples;
    one.data=adcInternalBuffer;
    two.data=NULL;
    instance->captureComplete(one,two);
    adc_dma_disable(ADC1);
}


/**
 */
void DSOADC::captureComplete(SampleSet &one, SampleSet &two)
{
    _captured.set1=one;
    _captured.set2=two;
    dmaSemaphore->giveFromInterrupt();
}

