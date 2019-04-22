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
 
 * Vref is using PWM mode for Timer4/Channel 3
 * 
 */

#include "dso_global.h"
#include "dso_adc_priv.h"

uint32_t DSOADC::adcInternalBuffer[ADC_INTERNAL_BUFFER_SIZE];

int dmaSpuriousInterrupt=0;
extern HardwareTimer Timer4;
/**
 */

#define analogInPin  PA0
#define triggerPin   PA8
#define vRefPin      PB8 // Trigger reference voltage

#define ADC_CR1_FASTINT 0x70000 // Fast interleave mode DUAL MODE bits 19-16
uint32_t convTime;
extern HardwareTimer Timer2;
adc_reg_map *adc_Register;
extern VoltageSettings vSettings[];
extern const float inputScale[];
DSOADC::TriggerMode triggerMode=DSOADC::Trigger_Both;
/**
 */
int requestedSamples;
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
  
  adc_Register=  PIN_MAP[PA0].adc_device->regs;
  

}
/**
 */
uint32_t DSOADC::getVCCmv()
{
    return vcc;
}

/**
 * 
 * @return 
 */
bool DSOADC::readCalibrationValue()
{
    #define NB_SAMPLE 16

   adc_Register = ADC1->regs;
   adc_Register->CR2 |= ADC_CR2_TSVREFE;    // enable VREFINT and temp sensor
   adc_Register->SMPR1 =  ADC_SMPR1_SMP17;  // sample ra

    
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
    return true;
}

/**
 * 
 */
void DSOADC::setADCs ()
{
 // 1 - Read VCC
   adc_Register = ADC1->regs;
   adc_Register->CR2 |= ADC_CR2_TSVREFE;    // enable VREFINT and temp sensor
   adc_Register->SMPR1 =  ADC_SMPR1_SMP17;  // sample ra

 // 2 - Setup ADC
    
  int pinMapADCin = PIN_MAP[analogInPin].adc_channel;
  adc_set_sample_rate(ADC1, ADC_SMPR_1_5); //=0,58uS/sample.  ADC_SMPR_13_5 = 1.08uS - use this one if Rin>10Kohm,
  adc_set_sample_rate(ADC2, ADC_SMPR_1_5);    // if not may get some sporadic noise. see datasheet.
  adc_set_prescaler(ADC_PRE_PCLK2_DIV_2);
   
  adc_set_reg_seqlen(ADC1, 1);
  
  adc_Register->SQR3 = pinMapADCin;
  adc_Register->CR2 |= ADC_CR2_CONT; // | ADC_CR2_DMA; // Set continuous mode and DMA
  adc_Register->CR1 |= ADC_CR1_FASTINT; // Interleaved mode
  adc_Register->CR2 |= ADC_CR2_SWSTART;
  
  ADC2->regs->CR2 |= ADC_CR2_CONT; // ADC 2 continuos
  ADC2->regs->SQR3 = pinMapADCin;
  
  pinMode(triggerPin,INPUT);
    
  Timer4.setPeriod(2000); // 5Khz pwm
  pinMode(vRefPin,PWM);
  pwmWrite(vRefPin,0);
  setTriggerMode(DSOADC::Trigger_Both);
}
/**
 * 
 * @param mode
 * @return 
 */
 bool DSOADC::setTriggerMode(TriggerMode mode)
 {
    ExtIntTriggerMode m;
    triggerMode=mode;
    switch(triggerMode)
    {
        case DSOADC::Trigger_Falling: m=FALLING;break;
        case DSOADC::Trigger_Rising:  m=RISING;break;
        case DSOADC::Trigger_Both:    m=CHANGE;break;
        default: xAssert(0);break;
    }
    // Hook trigger interrupt  
    attachInterrupt(triggerPin,TriggerInterrupt,m );
 }
 /**
  * 
  * @param ratio
  * @return 
  */
 bool DSOADC::setVrefPWM(int ratio)
 {
     pwmWrite(vRefPin,ratio);
 }
 
 /**
  * 
  * @return 
  */
 bool     DSOADC::getTriggerState()
 {
     return !!digitalRead(PA8);
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
bool    DSOADC::prepareDMASampling (adc_smp_rate rate,adc_prescaler scale)
{    

    setTimeScale(rate,scale);
    return true;
}
/**
 * 
 * @param count
 * @return 
 */
bool DSOADC::getSamples(FullSampleSet &fullSet)
{
    if(!dmaSemaphore->take(10000))
        return false;
    fullSet=_captured;
    return true;
}
 

// Grab the samples from the ADC
// Theoretically the ADC can not go any faster than this.
//
// According to specs, when using 72Mhz on the MCU main clock,the fastest ADC capture time is 1.17 uS. As we use 2 ADCs we get double the captures, so .58 uS, which is the times we get with ADC_SMPR_1_5.
// I think we have reached the speed limit of the chip, now all we can do is improve accuracy.
// See; http://stm32duino.com/viewtopic.php?f=19&t=107&p=1202#p1194


bool DSOADC::startDMASampling (int count)
{
  // This loop uses dual interleaved mode to get the best performance out of the ADCs
  //
    

  if(count>ADC_INTERNAL_BUFFER_SIZE/2)
        count=ADC_INTERNAL_BUFFER_SIZE/2;
  

  
  requestedSamples=count;  
  convTime=micros();
  dma_init(DMA1);
  dma_attach_interrupt(DMA1, DMA_CH1, DMA1_CH1_Event);
  
  dma_setup_transfer(DMA1, DMA_CH1, &ADC1->regs->DR, DMA_SIZE_32BITS, adcInternalBuffer, DMA_SIZE_32BITS, (DMA_MINC_MODE | DMA_TRNS_CMPLT));// Receive buffer DMA
  dma_set_num_transfers(DMA1, DMA_CH1, requestedSamples );
  adc_dma_enable(ADC1);
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
 * 
 */
void SPURIOUS_INTERRUPT()
{
    dmaSpuriousInterrupt++;
}

/**
* @brief Disable DMA requests
* @param dev ADC device on which to disable DMA requests
*/

void DSOADC::adc_dma_disable(const adc_dev * dev) 
{
  bb_peri_set_bit(&dev->regs->CR2, ADC_CR2_DMA_BIT, 0);
  dma_attach_interrupt(DMA1, DMA_CH1, SPURIOUS_INTERRUPT);
}
/**
 * 
 */
void DSOADC::DMA1_CH1_Event() 
{
    SampleSet one,two;
    two.samples=0;
    one.samples=requestedSamples;
    one.data=adcInternalBuffer;
    two.data=NULL;
    instance->captureComplete(true,one,two);
    adc_dma_disable(ADC1);
}

void DSOADC::TriggerInterrupt()
{
    triggered=true;
}

/**
 */
void DSOADC::captureComplete(bool shift,SampleSet &one, SampleSet &two)
{
    convTime=micros()-convTime;
    _captured.set1=one;
    _captured.set2=two;
    _captured.shifted=shift;
    dmaSemaphore->giveFromInterrupt();
}
/**
 * \bried cleanup already captured stuff
 */
void DSOADC::clearCapturedData()
{
   
}

/**
 * 
 */
void Oopps()
{
    while(1)
    {
        
    };
}

#include "dso_adc_fast_trigger.cpp"