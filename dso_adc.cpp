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

#include <Wire.h>
#include "SPI.h"
#include "MapleFreeRTOS1000.h"
#include "MapleFreeRTOS1000_pp.h"
#include "dso_adc.h"

// Analog input
#define ANALOG_MAX_VALUE 4096
// Samples - depends on available RAM 6K is about the limit on an STM32F103C8T6
// Bear in mind that the ILI9341 display is only able to display 240x320 pixels, at any time but we can output far more to the serial port, we effectively only show a window on our samples on the TFT.
# define maxSamples 256 //1024*6

const int8_t analogInPin = PA0;   // Analog input pin: any of LQFP44 pins (PORT_PIN), 10 (PA0), 11 (PA1), 12 (PA2), 13 (PA3), 14 (PA4), 15 (PA5), 16 (PA6), 17 (PA7), 18 (PB0), 19  (PB1)
xBinarySemaphore  *dmaSemaphore;
DSOADC *instance=NULL;
// Array for the ADC data
//uint16_t dataPoints[maxSamples];
int requestedSamples;
uint32_t *currentSamplingBuffer=NULL;
uint32_t vcc; // power Supply in mv
#define ADC_CR1_FASTINT 0x70000 // Fast interleave mode DUAL MODE bits 19-16




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
   vcc=0;
   for(int i=0;i<8;i++)
   {
       delay(10);   
       vcc+= (1200 * 4096) / adc_read(ADC1, 17); 
   }
   vcc/=8;
    
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
    capturedBuffers.addFromIsr(currentSamplingBuffer);
    currentSamplingBuffer=NULL;
    dmaSemaphore->giveFromInterrupt();
}
