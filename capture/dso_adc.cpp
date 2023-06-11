/*
 *  (C) 2021 MEAN00 fixounet@free.fr
 *  See license file
 *
 *  For now it works like this :
 *      Actively scan to see we are "OUT" of the trigger area
 *      When you are in, look for WD triggering in polling mode
 *      When the WD trigger arm a delay timer to have some samples after the trigger
 *      When the delay timer triggers, stop the DMA and warn the client
 *
 */
#include "dso_adc.h"
#include "../private_include/lnADC_priv.h"
#include "../private_include/lnPeripheral_priv.h"
#include "lnADC.h"
#include "lnArduino.h"
#include "lnCpuID.h"
#include "lnDma.h"
#include "lnTimer.h"

// statistics
static int dmaFull = 0, dmaHalf = 0, dmaCount = 0, timerArmed = 0;
;

static lnDSOAdc *_currentInstance = NULL;
static void delayIrq_(void *a);

//------------------------------------------------------------------

/**
 *
 * @param instance
 */
lnDSOAdc::lnDSOAdc(int instance, int timer, int channel)
    : lnBaseAdc(instance),
      _dma(lnDMA::DMA_PERIPH_TO_MEMORY, lnAdcDesc[_instance].dmaEngine, lnAdcDesc[_instance].dmaChannel, 32, 16),
      _delayTimer(1, 0)
{
    xAssert(instance == 0); // we need DMA so only ADC0
    _channel = channel;
    _timer = timer;
    _fq = -1;
    _adcTimer = NULL;
    _currentInstance = this;
    lnDisableInterrupt(LN_IRQ_ADC0_1);
    _dma.setPriority(lnDMA::DMA_PRIORITY_ULTRA_HIGH);
#warning HARDCODED
    lnIrqSetPriority((LnIRQ)LN_IRQ_DMA0_Channel0, 2); // more urgent than default
    lnIrqSetPriority(LN_IRQ_TIMER5, 3);
    _delayTimer.setInterrupt(delayIrq_, this);
    _cb = NULL;
}

/**
 *
 * @param a
 */
void delayIrq_(void *a)
{
    lnDSOAdc *adc = (lnDSOAdc *)a;
    adc->delayIrq();
}

/**
 *
 * @return
 */
void lnDSOAdc::delayIrq()
{
    // stop ADC
    LN_ADC_Registers *adc = lnAdcDesc[_instance].registers;
    adc->CTL1 &= ~(LN_ADC_CTL1_ADCON + LN_ADC_CTL1_DMA + LN_ADC_CTL1_CTN);
    if (_captureCb)
        _captureCb(_nbSamples, lnDMA::DMA_INTERRUPT_HALF, _triggerLocation);
}

/**
 *
 */
lnDSOAdc::~lnDSOAdc()
{
    if (_adcTimer)
        delete _adcTimer;
    _adcTimer = NULL;
}

/**
 *
 * @param timer
 * @param channel
 * @param fq
 * @param overSamplingLog2 , i.e. 0=>1; 1=>2;....;4=>16 etc.. The frequency will  be adjusted already for OVR
 * @return
 */
bool lnDSOAdc::setSource(int fq, lnPin pin, lnADC_DIVIDER divider, lnADC_CYCLES cycles, int overSamplingLog2)
{
    LN_ADC_Registers *adc = lnAdcDesc[_instance].registers;
    lnPinMode(pin, lnADC_MODE);
    _pin = pin;
    // overSamplingLog2=0;
    bool gd32 = lnCpuID::vendor() == lnCpuID::LN_MCU_GD32;
    _fq = fq;
    if (gd32)
    {
        _fq <<= overSamplingLog2;
    }

    int source = -1;
    int timerId = -1, timerChannel = -1;

    lnBaseAdc::setup();
    adc->CTL1 &= ~LN_ADC_CTL1_ADCON; // disable ADC so we can keep configuring it

#define SETTIM(c, a, b)                                                                                                \
    case a * 10 + b:                                                                                                   \
        source = LN_ADC_CTL1_ETSRC_SOURCE_##c;                                                                         \
        timerId = a;                                                                                                   \
        timerChannel = b;                                                                                              \
        break;
    switch (_timer * 10 + _channel)
    {
        SETTIM(T0CH0, 0, 0)
        SETTIM(T0CH1, 0, 1)
        SETTIM(T0CH2, 0, 2)
        SETTIM(T1CH1, 1, 1)
        SETTIM(T2TRGO, 2, 0)
        SETTIM(T3CH3, 3, 3)
    default:
        xAssert(0);
        break;
    }
    // set source
    uint32_t src = adc->CTL1 & LN_ADC_CTL1_ETSRC_MASK;
    src |= LN_ADC_CTL1_ETSRC_SET(source);
    adc->CTL1 = src | LN_ADC_CTL1_ETERC;
    //
    if (_adcTimer)
        delete _adcTimer;
    _adcTimer = new lnAdcTimer(timerId, timerChannel);
    _adcTimer->setPwmFrequency(_fq); // wtf

    // add our channel(s)
    // We go for single channel
    adc->RSQS[0] = adcChannel(pin);
    adc->RSQS[1] = 0;
    adc->RSQS[2] = 0;
    if (gd32)
    {
        if (!overSamplingLog2)
        {
            adc->OVRS = 0; // no oversampling
        }
        else
        {
            uint32_t ovr = 0;
            ovr |= LN_ADC_OVRS_OVSS_SET(overSamplingLog2);
            ovr |= LN_ADC_OVRS_OVSR_SET((overSamplingLog2 - 1));
            ovr |= LN_ADC_OVRS_OVSR_TOVS;
            ovr |= LN_ADC_OVRS_OVSEN;
            adc->OVRS = ovr;
        }
    }
    adc->CTL1 &= ~LN_ADC_CTL1_CTN;   // not yet
    adc->CTL1 &= ~LN_ADC_CTL1_DMA;   // not yet
    adc->CTL0 &= ~LN_ADC_CTL0_SM;    // not scan mode
    adc->CTL0 &= ~LN_ADC_CTL0_DISRC; // not dicontinuous

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
    lnDSOAdc *me = (lnDSOAdc *)t;
    me->dmaDone();
}
/**
 *
 */
void lnDSOAdc::dmaDone()
{
    LN_ADC_Registers *adc = lnAdcDesc[_instance].registers;
    _adcTimer->disable();
    // cleanup
    adc->CTL1 &= ~LN_ADC_CTL1_DMA;
    adc->CTL1 &= ~LN_ADC_CTL1_CTN;
    adc->CTL1 &= ~LN_ADC_CTL1_ADCON;
    // invoke CB
    if (_captureCb)
        _captureCb(_nbSamples, false, 0);
}
/**
 *
 * @param t
 * @param typ
 */
void lnDSOAdc::dmaTriggerDone_(void *t, lnDMA::DmaInterruptType typ)
{
    lnDSOAdc *me = (lnDSOAdc *)t;
    me->dmaTriggerDone(typ);
}
/**
 *
 * @param typ
 */
void lnDSOAdc::armTimer()
{
    _state = TRIGGERED;
    //--- ARM delay----------
    _dma.setInterruptMask(false, false);
    int currentSample = _nbSamples - _dma.getCurrentCount();
    timerArmed++;
#if 0
    delayIrq();
#else
    _delayTimer.arm(_cb->getDelayUs(currentSample, _triggerLocation, _nbSamples)); //
#endif
}
/**
 *
 * @param typ
 */
void lnDSOAdc::dmaTriggerDone(lnDMA::DmaInterruptType typ)
{
    LN_ADC_Registers *adc = lnAdcDesc[_instance].registers;
    //---
    dmaCount++;
    int scan = _nbSamples / 2;
    int beginIndex;
    uint16_t *start;
    switch (typ)
    {
    case lnDMA::DMA_INTERRUPT_HALF:
        start = _output;
        beginIndex = 0;
        xAssert(dmaCount & 1);
        dmaHalf++;
        break;
    case lnDMA::DMA_INTERRUPT_FULL:
        start = _output + scan;
        beginIndex = scan;
        dmaFull++;
        xAssert(!(dmaCount & 1));
        break;
    default:
        xAssert(0);
        break;
    }

    switch (_state)
    {
    case WARMUP: {
        xAssert(typ == lnDMA::DMA_INTERRUPT_HALF);
        _state = ARMING;
        adc->STAT &= ~LN_ADC_STAT_WDE;
        return;
    }
    break;
    case ARMING: {
        adc->STAT &= ~LN_ADC_STAT_WDE;
        int index;
        // Check we are NOT in the trigger area
        if (!_cb->lookup(ARMING, start, scan, index))
            return;
        start += index;
        beginIndex += index;
        scan -= index;
        _state = ARMED;
        if (_cb->lookup(ARMED, start, scan, index))
        {
            _triggerLocation = beginIndex + index;
            armTimer();
            return;
        }
        // not in that block, arm Watchdog

        return;

    } // no break ! = NOT A MISTALE
    case ARMED: {
        if (!(adc->STAT & LN_ADC_STAT_WDE))
            return; // no watchdog, no need to check

        int index;
        if (!_cb->lookup(ARMED, start, scan, index))
            return;

        _triggerLocation = beginIndex + index;
        armTimer();
    }
    break;
    default:
        xAssert(0);
        break;
    }
}

/**
 *
 * @param n
 * @param output
 * @return
 */
bool lnDSOAdc::startTriggeredDma(int n, uint16_t *output)
{
    _output = output;
    _nbSamples = n;
    dmaCount = 0;
    LN_ADC_Registers *adc = lnAdcDesc[_instance].registers;
    xAssert(_fq > 0);
    // Program DMA
    _state = IDLE;
    adc->CTL1 &= ~LN_ADC_CTL1_ADCON;
    // --Setup watchdog --

    uint32_t ctl0 = adc->CTL0;

    ctl0 |= LN_ADC_CTL0_WDSC;   // watchdog on a single channel
    ctl0 &= ~LN_ADC_CTL0_WDEIE; // enable watchdog interrupt
    ctl0 |= LN_ADC_CTL0_RWDEN;  // enable watchdog on regular
    ctl0 &= ~0x1f;
    ctl0 |= adcChannel(_pin);
    adc->CTL0 = ctl0;

    lnDisableInterrupt(LN_IRQ_ADC0_1);

    _dma.beginTransfer();
    _dma.attachCallback(lnDSOAdc::dmaTriggerDone_, this);

    // Go for the "safe" area
    int mn, mx;

    xAssert(_cb);
    _cb->getWatchdog(lnDSOAdc::ARMING, mn, mx);
    adc->WDHT = mx;
    adc->WDLT = mn;
    // Enable watchdog
    // adc->CTL0 |=LN_ADC_CTL0_WDEIE;
    // Circular mode + both interrupt
    _dma.doPeripheralToMemoryTransferNoLock(n, (uint16_t *)output, (uint16_t *)&(adc->RDATA), true, true);
    // go !
    adc->CTL1 &= ~LN_ADC_CTL1_CTN;
    adc->CTL1 |= LN_ADC_CTL1_DMA;

    // Set the trigger

    // We  want DMA interrupt to start the engine
    _dma.setInterruptMask(true, true);
    adc->STAT &= ~LN_ADC_STAT_WDE; // clear watchdog flag
    _adcTimer->enable();
    _state = WARMUP;
    // go !
    adc->CTL1 |= LN_ADC_CTL1_ADCON;
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
bool lnDSOAdc::startDmaTransfer(int n, uint16_t *output)
{
    LN_ADC_Registers *adc = lnAdcDesc[_instance].registers;
    xAssert(_fq > 0);
    lnDisableInterrupt(LN_IRQ_ADC0_1);
    uint32_t ctl0 = adc->CTL0;
    ctl0 &= ~LN_ADC_CTL0_WDSC;  // watchdog on a single channel
    ctl0 &= ~LN_ADC_CTL0_WDEIE; // disable watchdog interrupt
    adc->CTL0 = ctl0;
    // Program DMA
    _nbSamples = n;
    _dma.setInterruptMask(true, false);
    _dma.beginTransfer();
    _dma.attachCallback(lnDSOAdc::dmaDone_, this);
    _dma.doPeripheralToMemoryTransferNoLock(n, (uint16_t *)output, (uint16_t *)&(adc->RDATA), false);
    // go !
    adc->CTL1 |= LN_ADC_CTL1_DMA;
    adc->CTL1 &= ~LN_ADC_CTL1_CTN;

    _adcTimer->enable();
    // go !
    adc->CTL1 |= LN_ADC_CTL1_ADCON;

    return true;
}
/**
 *
 */
void lnDSOAdc::endCapture()
{
    _dma.endTransfer(); // This is incorrect, should be in its own task
}

void lnDSOAdc::stopCapture()
{
    LN_ADC_Registers *adc = lnAdcDesc[_instance].registers;
    adc->CTL1 &= ~(LN_ADC_CTL1_DMA + LN_ADC_CTL1_CTN + LN_ADC_CTL1_ADCON);
    adc->CTL1 &= ~LN_ADC_CTL1_CTN;
    _adcTimer->disable();
    _dma.cancelTransfer();
}
// EOF
