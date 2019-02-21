/**
 * Derived from https://github.com/pingumacpenguin/STM32-O-Scope/blob/master/STM32-O-Scope.ino
 */
/*.
(c) Andrew Hull - 2015
STM32-O-Scope - aka "The Pig Scope" or pigScope released under the GNU GENERAL PUBLIC LICENSE Version 2, June 1991
https://github.com/pingumacpenguin/STM32-O-Scope
Adafruit Libraries released under their specific licenses Copyright (c) 2013 Adafruit Industries.  All rights reserved.
*/
/**
 We use PA0 as input pin
 * DMA1, channel 0
 
 */

#include "dso_global.h"

/**
 */
#define maxSamples   (360) //1024*6
#define analogInPin  PA0
#define ADC_CR1_FASTINT 0x70000 // Fast interleave mode DUAL MODE bits 19-16
uint32_t convTime;
//
// this is 1/Gain for each range
// i.e. attenuation
//
const float inputScale[16]={
  /*[0] */  0, // GND
  /*[1] */  6.11/100., // /1  *14
  /*[2] */  12.19/100., // /2  *7
  /*[3] */  24.39/100., // /4  *3.5
  /*[4] */  61.64/100., // /10  *1.4
  /*[5] */  125./100., // /20  *0.8
  /*[6] */  243.9/100., // /40    *0.4 100 mv
    
  /*[7] */  6.11, // /1  /6.1   200mv
  /*[8] */  12.19, //2      /12  500mv
  /*[9] */  24.39,  //4     /24  1V
  /*[a] */  61.64,  //10     /60
  /*[b] */  125, //20     /125
  /*[c] */  250 // 40    /250
};
/*
 * Partially filled global gain array
 * Remaining columns will be filled at runtime
 */
VoltageSettings vSettings[11]=
{
    {"1mv",     1,  24000.},
    {"5mv",     2,  4800.},
    {"10mv",    3,  2400.},
    {"20mv",    4 , 1200.},
    {"50mv",    5,  480.},
    {"100mv",   6,  240.},
    {"200mv",   7,  120.},
    {"500mv",   8,  48.},
    {"1v",      9,  24.},
    {"2v",      10, 12.},
    {"5v",      11, 4.8}
};
/**
 These the time/div settings, it is computed to maximume accuracy 
 * and sample a bit too fast, so that we can decimate it
 *  */
TimeSettings tSettings[6]
{
    {"10us",    ADC_PRE_PCLK2_DIV_2,ADC_SMPR_1_5,   4390},
    {"25us",    ADC_PRE_PCLK2_DIV_2,ADC_SMPR_13_5,  5909},
    {"50us",    ADC_PRE_PCLK2_DIV_2,ADC_SMPR_55_5,  4496},
    {"100us",   ADC_PRE_PCLK2_DIV_4,ADC_SMPR_55_5,  4517},
    {"500us",   ADC_PRE_PCLK2_DIV_4,ADC_SMPR_239_5, 6095},
    {"1ms",     ADC_PRE_PCLK2_DIV_8,ADC_SMPR_239_5, 6095}
};


/**
 */
int requestedSamples;
uint32_t *currentSamplingBuffer=NULL;
uint32_t vcc; // power Supply in mv

xBinarySemaphore  *dmaSemaphore;
DSOADC             *instance=NULL;



/**
 * 
 */
DSOADC::DSOADC()
{
  instance=this;
  adc_calibrate(ADC1);
  adc_calibrate(ADC2);
  setADCs (); //Setup ADC peripherals for interleaved continuous mode.
  // Set up our sensor pin(s)
  pinMode(analogInPin, INPUT_ANALOG);
  dmaSemaphore=new xBinarySemaphore;
  for(int i=0;i<SAMPLING_QUEUE_SIZE;i++)
  {
      uint32_t *data=new uint32_t[maxSamples];
      if(!data)
      {
          xAssert(0);
      }
      availableBuffers.addFromIsr(data);
  }
}
/**
 */
uint32_t DSOADC::getVCCmv()
{
    return vcc;
}
/**
 * 
 */
void DSOADC::setADCs ()
{
  
 // 1 - Read VCC
   adc_reg_map *regs = ADC1->regs;
   regs->CR2 |= ADC_CR2_TSVREFE;    // enable VREFINT and temp sensor
   regs->SMPR1 =  ADC_SMPR1_SMP17;  // sample ra
#define NB_SAMPLE 16
   float fvcc=0;
   for(int i=0;i<NB_SAMPLE;i++)
   {
       delay(10);   
       fvcc+=  adc_read(ADC1, 17); 
   }
    fvcc=(1200. * 4096.*NB_SAMPLE) /fvcc;   
    fvcc=3380;
    vcc=(int)fvcc;
    
    
    // 1b fill up the conversion table
    for(int i=0;i<11;i++)
    {
        vSettings[i].offset=calibrationDC[i+1];
        vSettings[i].multiplier=inputScale[i+1]*fvcc/4096000.;
    }
 // 2 - Setup ADC
    
  int pinMapADCin = PIN_MAP[analogInPin].adc_channel;
  adc_set_sample_rate(ADC1, ADC_SMPR_1_5); //=0,58uS/sample.  ADC_SMPR_13_5 = 1.08uS - use this one if Rin>10Kohm,
  adc_set_sample_rate(ADC2, ADC_SMPR_1_5);    // if not may get some sporadic noise. see datasheet.
  adc_set_prescaler(ADC_PRE_PCLK2_DIV_2);
   
  //  adc_reg_map *regs = dev->regs;
  adc_set_reg_seqlen(ADC1, 1);
  ADC1->regs->SQR3 = pinMapADCin;
  ADC1->regs->CR2 |= ADC_CR2_CONT; // | ADC_CR2_DMA; // Set continuous mode and DMA
  ADC1->regs->CR1 |= ADC_CR1_FASTINT; // Interleaved mode
  ADC1->regs->CR2 |= ADC_CR2_SWSTART;

  ADC2->regs->CR2 |= ADC_CR2_CONT; // ADC 2 continuos
  ADC2->regs->SQR3 = pinMapADCin;
}
/**
 * 
 * @param timeScaleUs
 * @return 
 */
 bool    DSOADC::setTimeScale(adc_smp_rate one, adc_prescaler two)
 {
    adc_set_sample_rate(ADC1, one); //=0,58uS/sample.  ADC_SMPR_13_5 = 1.08uS - use this one if Rin>10Kohm,
    adc_set_prescaler(two);

     return true;
 }
 /**
  * 
  * @param count
  * @return 
  */
bool    DSOADC::initiateSampling (int count)
{    
    if(!capturedBuffers.empty())
        return true; // We have data !
    
    uint32_t *bfer=availableBuffers.take();
    if(!bfer) return false;
    
    return startSampling(count,bfer);
    
}
   
/**
 * 
 * @param count
 * @return 
 */
uint32_t *DSOADC::getSamples(int &count)
{
again:
    noInterrupts();
    uint32_t *data=capturedBuffers.takeFromIsr();
    if(data)
    {
        count=requestedSamples;
        interrupts();
        return data;
    }
    interrupts();
    dmaSemaphore->take(10000); // 10 sec timeout
    dma_disable(DMA1, DMA_CH1); //End of trasfer, disable DMA and Continuous mode.
    count=requestedSamples;
    data=capturedBuffers.take();
    if(!data) goto again;
    return data;
}
/**
 * 
 * @param buffer
 * @return 
 */
void     DSOADC::reclaimSamples(uint32_t *buffer)
{
    availableBuffers.add(buffer);
}
 
 
// Grab the samples from the ADC
// Theoretically the ADC can not go any faster than this.
//
// According to specs, when using 72Mhz on the MCU main clock,the fastest ADC capture time is 1.17 uS. As we use 2 ADCs we get double the captures, so .58 uS, which is the times we get with ADC_SMPR_1_5.
// I think we have reached the speed limit of the chip, now all we can do is improve accuracy.
// See; http://stm32duino.com/viewtopic.php?f=19&t=107&p=1202#p1194

bool DSOADC::startSampling (int count,uint32_t *buffer)
{
  // This loop uses dual interleaved mode to get the best performance out of the ADCs
  //

  if(count>maxSamples)
        count=maxSamples;
    
  requestedSamples=count;
  currentSamplingBuffer=buffer;
  convTime=micros();
  dma_init(DMA1);
  dma_attach_interrupt(DMA1, DMA_CH1, DMA1_CH1_Event);

  adc_dma_enable(ADC1);
  dma_setup_transfer(DMA1, DMA_CH1, &ADC1->regs->DR, DMA_SIZE_32BITS, buffer, DMA_SIZE_32BITS, (DMA_MINC_MODE | DMA_TRNS_CMPLT));// Receive buffer DMA
  dma_set_num_transfers(DMA1, DMA_CH1, requestedSamples );

  dma_enable(DMA1, DMA_CH1); // Enable the channel and start the transfer.
  return true;
}

/**
* @brief Enable DMA requests
* @param dev ADC device on which to enable DMA requests
*/

void DSOADC::adc_dma_enable(const adc_dev * dev) 
{
  bb_peri_set_bit(&dev->regs->CR2, ADC_CR2_DMA_BIT, 1);
}


/**
* @brief Disable DMA requests
* @param dev ADC device on which to disable DMA requests
*/

void DSOADC::adc_dma_disable(const adc_dev * dev) 
{
  bb_peri_set_bit(&dev->regs->CR2, ADC_CR2_DMA_BIT, 0);
}
/**
 * 
 */
void DSOADC::DMA1_CH1_Event() 
{
    instance->captureComplete();
}
/**
 */
void DSOADC::captureComplete()
{
    convTime=micros()-convTime;
    capturedBuffers.addFromIsr(currentSamplingBuffer);
    currentSamplingBuffer=NULL;
    dmaSemaphore->giveFromInterrupt();
}

