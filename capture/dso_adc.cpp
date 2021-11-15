/*
 *  (C) 2021 MEAN00 fixounet@free.fr
 *  See license file
 */
#include "lnArduino.h"
#include "../private_include/lnPeripheral_priv.h"
#include "lnADC.h"
#include "../private_include/lnADC_priv.h"
#include "lnTimer.h"
#include "lnDma.h"
#include "dso_adc.h"
#include "lnCpuID.h"

static lnDSOAdc *_currentInstance=NULL;
extern lnDSOAdc::lnDSOADC_State DSOCapture_getWatchdog(lnDSOAdc::lnDSOADC_State state, int &mn, int &mxv);
//------------------------------------------------------------------
void dsoAdcIRq()
{
    xAssert(_currentInstance);
    _currentInstance->irqHandler();
}
 
/**
 * 
 * @param instance
 */
lnDSOAdc::lnDSOAdc(int instance,int timer, int channel)  : lnBaseAdc(instance), 
        _dma( lnDMA::DMA_PERIPH_TO_MEMORY,   lnAdcDesc[_instance].dmaEngine, lnAdcDesc[_instance].dmaChannel,  32,16)
{
    xAssert(instance==0); // we need DMA so only ADC0
    _channel=channel;
    _timer=timer;
    _fq=-1;
    _adcTimer=NULL;
    _currentInstance=this;
    lnDisableInterrupt(LN_IRQ_ADC0_1);
    lnSetInterruptHandler(LN_IRQ_ADC0_1, dsoAdcIRq);
}
/**
 * 
 */
lnDSOAdc::~lnDSOAdc()
{
    if(_adcTimer) delete _adcTimer;
    _adcTimer=NULL;
}
/**
 *  // Normally we can get here only when watchdog interrupt happens
 */
void lnDSOAdc::irqHandler(void)
{
    LN_ADC_Registers *adc=lnAdcDesc[_instance].registers;
    switch(_state)
    {
        case ARMING: // Prepare for trigger, we reached an area that can trigger, reprogram watchdog for actual trigger
        {
                  int mn=2048,mx=2048;      
                  _state=DSOCapture_getWatchdog(_state,mn,mx); // this is ugly
                  // We need to stop DMA and restart the whole thing 
                  _dma.pause();
                  adc->STAT &=~LN_ADC_STAT_WDE; 
                  adc->CTL1&=~LN_ADC_CTL1_ADCON;
                  adc->WDHT=mx; // can we change the watchdog on the fly ????
                  adc->WDLT=mn;
                  _dma.resume();
                  adc->CTL1|=LN_ADC_CTL1_ADCON; // re-enable ADC 
                  return;
        }
                  break;
        case ARMED: // the watchdog is an actual trigger here
        {
              _triggerLocation=_nb-_dma.getCurrentCount();
              adc->CTL0 &=~LN_ADC_CTL0_WDEIE; 
              adc->STAT &=~LN_ADC_STAT_WDE; 
              _state=TRIGGERED;
              return;
        }
             break;
        default:
              xAssert(0);
              break;                  
    }   
}

/**
 * 
 * @param timer
 * @param channel
 * @param fq
 * @param overSamplingLog2 , i.e. 0=>1; 1=>2;....;4=>16 etc.. The frequency will  be adjusted already for OVR
 * @return 
 */
bool     lnDSOAdc::setSource(  int fq,lnPin pin,lnADC_DIVIDER divider,lnADC_CYCLES cycles, int overSamplingLog2)
{
    LN_ADC_Registers *adc=lnAdcDesc[_instance].registers;
    _pin=pin;
    //overSamplingLog2=0;
    
    _fq=fq;    
    if(lnCpuID::vendor()==lnCpuID::LN_MCU_GD32)
    {        
     _fq<<= overSamplingLog2;  
    }
   
    int source=-1;
    int timerId=-1,timerChannel=-1;
    
    lnBaseAdc::setup();
    adc->CTL1&=~LN_ADC_CTL1_ADCON; // disable ADC so we can keep configuring it
    
#define SETTIM(c,a,b) case a*10+b: source=LN_ADC_CTL1_ETSRC_SOURCE_##c;timerId=a;timerChannel=b;break;
    switch(_timer*10+_channel)
    {
        SETTIM(T0CH0,0,0)
        SETTIM(T0CH1,0,1)
        SETTIM(T0CH2,0,2)
        SETTIM(T1CH1,1,1)
        SETTIM(T2TRGO,2,0)
        SETTIM(T3CH3,3,3)
        default:
            xAssert(0);
            break;
    }
    // set source
    uint32_t src=adc->CTL1&LN_ADC_CTL1_ETSRC_MASK;
    src |=LN_ADC_CTL1_ETSRC_SET(source);
    adc->CTL1=src | LN_ADC_CTL1_ETERC;
    //
    if(_adcTimer) delete _adcTimer;
    _adcTimer=new lnAdcTimer(timerId, timerChannel);
    _adcTimer->setPwmFrequency(_fq); // wtf
    
    // add our channel(s)
    // We go for single channel
    adc->RSQS[0]=adcChannel(pin);
    adc->RSQS[1]=0;
    adc->RSQS[2]=0;
    if(lnCpuID::vendor()==lnCpuID::LN_MCU_GD32)
    {
        if(!overSamplingLog2)
        {
             adc->OVRS=0; // no oversampling
        }else
        {
            uint32_t ovr=0;        
            ovr|=LN_ADC_OVRS_OVSS_SET(overSamplingLog2);
            ovr|=LN_ADC_OVRS_OVSR_SET((overSamplingLog2-1));
            ovr|=LN_ADC_OVRS_OVSR_TOVS;
            ovr|=LN_ADC_OVRS_OVSEN;
            adc->OVRS=ovr;
        }
    }
    adc->CTL1&=~LN_ADC_CTL1_CTN;    // not yet
    adc->CTL1&=~LN_ADC_CTL1_DMA;    // not yet
    adc->CTL0&=~LN_ADC_CTL0_SM; // not scan mode
    adc->CTL0&=~LN_ADC_CTL0_DISRC; // not dicontinuous
    
    lnPeripherals::setAdcDivider(divider);       
    setSmpt(cycles);    
    return true;
}
/**
 * 
 * @param t
 */
void lnDSOAdc::dmaDone_(void *t, lnDMA::DmaInterruptType typ)
{
    lnDSOAdc *me=(lnDSOAdc *)t;
    me->dmaDone();
}
/**
 * 
 */
void lnDSOAdc::dmaDone()
{ 
    LN_ADC_Registers *adc=lnAdcDesc[_instance].registers;
    _adcTimer->disable();
    // cleanup
    adc->CTL1&=~LN_ADC_CTL1_DMA;
    adc->CTL1&=~LN_ADC_CTL1_CTN;
    adc->CTL1&=~LN_ADC_CTL1_ADCON;
    // invoke CB
    if(_captureCb)
        _captureCb(_nbSamples,false);
}
/**
 * 
 * @param t
 * @param typ
 */
void lnDSOAdc::dmaTriggerDone_(void *t, lnDMA::DmaInterruptType typ)
{
    lnDSOAdc *me=(lnDSOAdc *)t;
    me->dmaTriggerDone(typ);
}
/**
 * 
 * @param typ
 */

void lnDSOAdc::dmaTriggerDone(lnDMA::DmaInterruptType typ)
{ 
  LN_ADC_Registers *adc=lnAdcDesc[_instance].registers;

  if(_state!=TRIGGERED || _dmaLoop<2)
  {  
    _dmaLoop++;
    return;        
  }
  bool stop=false;
  uint32_t offset=(_triggerLocation*4)/_nb;

    switch(typ)
    {
        case lnDMA::DMA_INTERRUPT_HALF:  if(offset!=1) stop=true;break;
        case lnDMA::DMA_INTERRUPT_FULL: if(offset!=3) stop=true;break;
        default: xAssert(0);break;
    }
    if(stop)
    {
      _adcTimer->disable();
      // cleanup
      adc->CTL1&=~LN_ADC_CTL1_DMA;
      adc->CTL1&=~LN_ADC_CTL1_CTN;
      adc->CTL1&=~LN_ADC_CTL1_ADCON;
      // invoke CB
      if(_captureCb)
          _captureCb(_nbSamples,typ==lnDMA::DMA_INTERRUPT_HALF);
    }   
   _dmaLoop++;
}

/**
 * 
 * @param n
 * @param output
 * @return 
 */
bool     lnDSOAdc::startTriggeredDma(int n,  uint16_t *output) 
{
    _output=output;
    _nb=n;
    LN_ADC_Registers *adc=lnAdcDesc[_instance].registers;       
    xAssert(_fq>0);
    // Program DMA
    _nbSamples=n;
    _state=IDLE;
    
    // --Setup watchdog --
    
    uint32_t ctl0=adc->CTL0;
    
    ctl0 |=LN_ADC_CTL0_WDSC;  // watchdog on a single channel
    ctl0 &=~LN_ADC_CTL0_WDEIE; // enable watchdog interrupt
    ctl0 |=LN_ADC_CTL0_RWDEN; // enable watchdog on regular
    ctl0 &=~0x1f;
    ctl0 |=adcChannel(_pin);
    adc->CTL0=ctl0;
    
    _dmaLoop=0;
    lnEnableInterrupt(LN_IRQ_ADC0_1);    
    
    _dma.beginTransfer();
    _dma.attachCallback(lnDSOAdc::dmaTriggerDone_,this);
    
    // Go for the "safe" area
    int mn,mx;
     adc->STAT &=~LN_ADC_STAT_WDE;  // clear watchdog flag
    _state=DSOCapture_getWatchdog(lnDSOAdc::IDLE,mn,mx);
     adc->WDHT=mx;
     adc->WDLT=mn;
     // Enable watchdog
     adc->CTL0 |=LN_ADC_CTL0_WDEIE; 
    // Circular mode + both interrupt
    _dma.doPeripheralToMemoryTransferNoLock(n, (uint16_t *)output,(uint16_t *)&( adc->RDATA),  true,true);
    // go !
    adc->CTL1&=~LN_ADC_CTL1_CTN;
    adc->CTL1|=LN_ADC_CTL1_DMA;
    _adcTimer->enable();    
    // go !
    adc->CTL1|=LN_ADC_CTL1_ADCON;
    return true;
}

/**
 * 
 * @param fq
 * @param nbSamplePerChannel
 * @param nbPins
 * @param pins
 * @param output
 * @return 
 */
bool     lnDSOAdc::startDmaTransfer(int n,  uint16_t *output) 
{
    LN_ADC_Registers *adc=lnAdcDesc[_instance].registers;       
    xAssert(_fq>0);
    lnDisableInterrupt(LN_IRQ_ADC0_1);
    uint32_t ctl0=adc->CTL0;
    ctl0 &=~LN_ADC_CTL0_WDSC; // watchdog on a single channel
    ctl0 &=~LN_ADC_CTL0_WDEIE; // disable watchdog interrupt
    adc->CTL0=ctl0;
    // Program DMA
    _nbSamples=n;
    _dma.beginTransfer();
    _dma.attachCallback(lnDSOAdc::dmaDone_,this);
    _dma.doPeripheralToMemoryTransferNoLock(n, (uint16_t *)output,(uint16_t *)&( adc->RDATA),  false);
    // go !
    adc->CTL1|=LN_ADC_CTL1_DMA;
    adc->CTL1&=~LN_ADC_CTL1_CTN;
    
    _adcTimer->enable();    
    // go !
    adc->CTL1|=LN_ADC_CTL1_ADCON;
    
    return true;
}
/**
 * 
 */
void      lnDSOAdc::endCapture()
{
      _dma.endTransfer();    // This is incorrect, should be in its own task
}


void lnDSOAdc:: stopCapture()
{
    LN_ADC_Registers *adc=lnAdcDesc[_instance].registers;      
    adc->CTL1&=~LN_ADC_CTL1_DMA;
    adc->CTL1&=~LN_ADC_CTL1_CTN;
    _dma.cancelTransfer(); 
}
// EOF