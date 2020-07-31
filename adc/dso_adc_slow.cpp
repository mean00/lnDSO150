/**
 
 *  This is the slow capture mode
 * i.e. we setup n Samples acquisition through DMA
 * and a timer interrupt grabs the result 
 */

//#include "dso_global.h"
#include "dso_adc.h"
#include "dso_adc_priv.h"
#include "fancyLock.h"
#include "myPwm.h"
/**
 */
uint32_t lastStartedCR2=0;
uint32_t lastStartedCR1=0;
uint32_t lastStartedSR;
//

CaptureState captureState=Capture_idle;
void dumpAdcRegs();

/**
 * \brief compute rate & scale so that the sampling frequency is more than double the ADC sampling fq
 * @param fq
 * @param scaler
 * @param rate
 * @return 
 */

typedef struct RateArray
{
    adc_smp_rate rate;
    int          rateInt;
};

const RateArray rateArrays[]={
    {  ADC_SMPR_1_5,1},
    {  ADC_SMPR_7_5,7},
    {  ADC_SMPR_13_5,13},
    {  ADC_SMPR_28_5,28},
    {  ADC_SMPR_41_5,41},
    {  ADC_SMPR_55_5,55},
    {  ADC_SMPR_71_5,71},
    {  ADC_SMPR_239_5,239}
};

bool DSOADC::frequencyToRateScale (int fq, DSOADC::Prescaler &scaler, adc_smp_rate &rate)
{
    if(fq>=500000)
    {
            rate=ADC_SMPR_1_5;
            scaler=DSOADC::ADC_PRESCALER_2;
            return true;
    }
    bool found=false;
    for(int s=4;s>=0 && !found;s--)
    {
        for(int ri=(sizeof(rateArrays)/sizeof(RateArray))-1; ri>=0 && !found;ri--)
        {
            int sampling=F_CPU/((s*2)*(12+rateArrays[ri].rateInt));
            if(sampling>2*fq)
            {
                found=true;
                scaler=(DSOADC::Prescaler)(s-1);
                rate=rateArrays[ri].rate;
                return true;
            }
        }
    }
    xAssert(0);
    return false;
}

/**
 * 
 */
void DSOADC::stopTimeCapture(void)
{
     ADC_TIMER.pause();
     adc_dma_disable(ADC1);
     allAdcsOnOff(false);
     timer_cc_disable(ADC_TIMER.c_dev(), ADC_TIMER_CHANNEL);
}
/**
 * 
 * @return 
 */
bool DSOADC::startDualTime()
{
    
  dumpAdcRegs(); 
    
  cr2=ADC1->regs->CR2;  
  cr2|=ADC_CR2_DMA;
  cr2&= ~(ADC_CR2_SWSTART+ADC_CR2_CONT);   
  ADC1->regs->CR2=cr2;
  setSourceInternal();   
  
  uint32_t   cr2=ADC2->regs->CR2;  
  cr2 &=~ (ADC_CR2_EXTSEL_SWSTART+ADC_CR2_CONT);
  ADC2->regs->CR2=cr2;
  
  setSourceInternal(ADC2)  ;
    
  dumpAdcRegs(); 
  return true;  
}

/**
 * 
 * @return 
 */
bool DSOADC::startDMATime()
{    
  cr2=ADC1->regs->CR2;  
  cr2&= ~(ADC_CR2_SWSTART+ADC_CR2_CONT);   
  ADC1->regs->CR2=cr2;
  setSourceInternal();   
  cr2|=ADC_CR2_DMA;    
  ADC1->regs->CR2=cr2;    
  return true;  
}
/**
 * 
 * @param count
 * @param buffer
 * @return 
 */
bool DSOADC::startInternalDmaSampling ()
{
  setupAdcDmaTransfer( requestedSamples,adcInternalBuffer, DMA1_CH1_Event,false );
  
  startDMATime();
  volatile uint32_t s =ADC1->regs->DR;
  s =ADC2->regs->DR;  
  lastStartedCR2=ADC1->regs->CR2;
  dumpAdcRegs();  
  ADC_TIMER.setCount(0);
  ADC_TIMER.resume();  
  return true;
}

/**
 * 
 * @param count
 * @param buffer
 * @return 
 */
bool DSOADC::startTimerSampling (int count)
{

   if(count>ADC_INTERNAL_BUFFER_SIZE)
        count=ADC_INTERNAL_BUFFER_SIZE;
    requestedSamples=count;

    FancyInterrupts::disable();    
    captureState=Capture_armed;       
    startInternalDmaSampling();   
    allAdcsOnOff(true);
    FancyInterrupts::enable();
    return true;
} 
/**
 * 
 * @param count
 * @return 
 */

uint32_t regs1[16];
uint32_t regs2[16];
void dumpAdcRegs()
{
    uint32_t *p1=(uint32_t *)ADC1->regs;
    uint32_t *p2=(uint32_t *)ADC2->regs;
     for(int i=0;i<15;i++)
    {
        regs1[i]=p1[i];
        regs2[i]=p2[i];
    } 
}
/**
 * 
 * @param count
 * @return 
 */
bool DSOADC::startDualTimeSampling (const int otherPin,int count, int preload)
{
    
    if(count>ADC_INTERNAL_BUFFER_SIZE)
        count=ADC_INTERNAL_BUFFER_SIZE;
    requestedSamples=count;
    
    FancyInterrupts::disable();    
    captureState=Capture_armed;   
    
    dumpAdcRegs();   

    volatile uint32_t s =ADC1->regs->DR;
    s =ADC2->regs->DR;
    
    ADC_TIMER.setCount(preload); 
    
    setupAdcDualDmaTransfer( otherPin, requestedSamples,(uint32_t *)adcInternalBuffer, DMA1_CH1_Event,false );
    startDualTime();
    ADC_TIMER.resume();  
    lastStartedCR2=ADC1->regs->CR2;
    FancyInterrupts::enable();
    return true;
} 

/**
 * 
 * @param fq
 * @param otherPin
 * @param rate
 * @param scale
 * @return 
 */
bool    DSOADC::prepareDualTimeSampling (int fq,int otherPin, adc_smp_rate rate,DSOADC::Prescaler  scale)
{  
    
    _dual=DSOADC::ADC_CAPTURE_DUAL_SIMULTANEOUS;
    ADC1->regs->CR1&=~ADC_CR1_DUALMASK;  
    ADC1->regs->CR1|=ADC_CR1_DUAL_REGULAR_SIMULTANEOUS;
    ADC2->regs->CR1&=~ADC_CR1_DUALMASK;      
    // not needed ADC2->regs->CR1|=ADC_CR1_DUAL_REGULAR_SIMULTANEOUS;
    
    dumpAdcRegs();   
    
    ADC2->regs->SQR3 = PIN_MAP[otherPin].adc_channel ;          
    ADC2->regs->CR2 &= ~ADC_CR2_CONT;
    ADC1->regs->CR2 &= ~ADC_CR2_CONT ;
    ADC1->regs->CR2 |= ADC_CR2_DMA;
    
    dumpAdcRegs();   
    
    adc_set_sample_rate(ADC2, rate); 

    // now set frequency
    return prepareTimerSampling(fq,false,rate,scale);
}

bool DSOADC::setupDualTimerSampling()
{
   ADC_TIMER.pause();
   setSource(ADC_SOURCE_TIMER);    
   _oldTimerFq=0;  
   return true;
}
/**
 * 
 * @param overFlow
 * @param scaler
 * @return 
 */
bool DSOADC::programTimer(int overFlow, int scaler)
{
#if 1
    pwmFromScalerAndOverflow(&ADC_TIMER, ADC_TIMER_CHANNEL,scaler,overFlow);
#else    
    ADC_TIMER.setPrescaleFactor(scaler);
    ADC_TIMER.setOverflow(overFlow);
    ADC_TIMER.setCompare(ADC_TIMER_CHANNEL,overFlow/2);
    timer_cc_enable(ADC_TIMER.c_dev(), ADC_TIMER_CHANNEL);
#endif    
    return true;
}

// EOF

