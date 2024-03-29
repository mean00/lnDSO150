/***************************************************
 STM32 duino based firmware for DSO SHELL/150
 *  * GPL v2
 * (c) mean 2019 fixounet@free.fr
 ****************************************************/
#include "dso_capture.h"
#include "dso_adc.h"
#include "lnArduino.h"
#include "lnCpuID.h"
#include "lnDso_fp.h"
#include "pinConfiguration.h"

#if 0
#define debug Logger
#else
#define debug(...)                                                                                                     \
    {                                                                                                                  \
    }
#endif

lnPin DSOCapture::_pin;
captureCb *DSOCapture::_cb;
int DSOCapture::_nb;
bool DSOCapture::_med;
int DSOCapture::_triggerLocation;
int DSOCapture::currentVoltageRange = 0;
DSOCapture::DSO_TIME_BASE DSOCapture::currentTimeBase = DSOCapture::DSO_TIME_BASE_1MS;
lnDSOAdc *DSOCapture::_adc;
uint16_t *internalAdcBuffer;
DSOCapture::captureState DSOCapture::_state = DSOCapture::CAPTURE_STOPPED;

float DSOCapture::_triggerVolt = 1.0;
int DSOCapture::_triggerAdc = 2048;
DSOCapture::TriggerMode DSOCapture::_triggerMode;
bool DSOCapture::_couplingModeIsAC = 0;
/**
 */
static const lnDSOADCCallbacks adcCaptureCb = {DSOCapture::getWatchdog, DSOCapture::lookup, DSOCapture::delay};

/**
 *
 * @return
 */
float DSOCapture::getTriggerVoltage()
{
    return _triggerVolt;
}
/**
 * This returns the time (in us) to let the delay timer run
 * so that we have 1/4 of the buffer full
 * @return
 */
int DSOCapture::delay(int currentLocation, int triggerLocation, int loopSize)
{
    //   timerBases[currentTimeBase].usToFillBuffer; is the time to fill 1/4 of the buffer
    //   ((x*DSO_CAPTURE_INTERNAL_BUFFER_SIZE)/(20*4)) // X/20 pixel/div => core clock, multiplied by BUFFER SIZE/4 =>
    //   time to fill 1/4 of the buffer
    int ratio = (currentLocation + loopSize - triggerLocation) & (loopSize - 1);
    // bufffer consumed in 1/8th of the buffer
    ratio = (8 * ratio + 4) / loopSize;
    if (ratio >= 2)
        return 20;                                          // Consumed already 2*1/8=1/4 of the buffer
    return 20 + timerBases[currentTimeBase].usToFillBuffer; // ask for 1/4 more
}

/**
 *
 *  Warning, we do something a bit fishy here
 *   1- We ask for 2 consecutive value that matches the condition
 *   2- There is a small hysteresis
 *   If we dont do that, the noise will be enough to trigger even if the condition is inverted
 *   i.e.
 *    \
 *     \/\   < a bit of noise here will trigger
 *        \
 */

#define MACRO_SEARCH(cond)                                                                                             \
    {                                                                                                                  \
        for (int i = 1; i < size && !f; i++)                                                                           \
        {                                                                                                              \
            if ((data[i] cond) && (data[i - 1] cond))                                                                  \
            {                                                                                                          \
                f = true;                                                                                              \
                index = i;                                                                                             \
                return true;                                                                                           \
            }                                                                                                          \
        }                                                                                                              \
        return false;                                                                                                  \
    }

/**
 * \brief search in the capture buffer to see if we have a trigger
 * /!\ it will miss stuff when it happens just at the boundary!
 * @param data
 * @param size
 * @param index
 * @return
 */
bool DSOCapture::lookup(lnDSOAdc::lnDSOADC_State state, uint16_t *data, int size, int &index)
{
    if (state == lnDSOAdc::ARMING)
    {
        bool f = false;
        switch (DSOCapture::getTriggerMode())
        {

        case DSOCapture::Trigger_Run:
            xAssert(0);
            break;
        case DSOCapture::Trigger_Both:
        case DSOCapture::Trigger_Rising: {
            int level = DSOCapture::_triggerAdc - 5;
            if (level < 0)
                level = 1;
            MACRO_SEARCH(< level)
        }
        break;
        case DSOCapture::Trigger_Falling: {
            int level = DSOCapture::_triggerAdc + 5;
            if (level > 4094)
                level = 4094;
            MACRO_SEARCH(> level)
        }
        break;
        }
        return false;
    }
    if (state == lnDSOAdc::ARMED)
    {
        bool f = false;
        switch (DSOCapture::getTriggerMode())
        {
        case DSOCapture::Trigger_Run:
            xAssert(0);
            break;
        case DSOCapture::Trigger_Both:
        case DSOCapture::Trigger_Rising: {
            int level = DSOCapture::_triggerAdc + 5;
            if (level > 4094)
                level = 4094;
            MACRO_SEARCH(>= level)
        }
        break;
        case DSOCapture::Trigger_Falling: {
            int level = DSOCapture::_triggerAdc - 5;
            if (level < 0)
                level = 1;
            MACRO_SEARCH(<= level)
        }
        break;
        }
        return false;
    }
    xAssert(0);
    return false;
}

/**
 *
 * @param current
 * @param mn
 * @param mxv
 * @return
 */
lnDSOAdc::lnDSOADC_State DSOCapture_getWatchdog(lnDSOAdc::lnDSOADC_State state, int &mn, int &mxv)
{
    return DSOCapture::getWatchdog(state, mn, mxv);
}
/**
 *
 * @param current
 * @param mn
 * @param mxv
 * @return
 */
lnDSOAdc::lnDSOADC_State DSOCapture::getWatchdog(lnDSOAdc::lnDSOADC_State state, int &mn, int &mx)
{
    switch (state)
    {
    case lnDSOAdc::IDLE: // first let's go to start state
        switch (_triggerMode)
        {
        case Trigger_Rising:
            mn = _triggerAdc;
            mx = 4095;
            return lnDSOAdc::ARMING;
            break;
        case Trigger_Falling:
            mn = 0;
            mx = _triggerAdc;
            return lnDSOAdc::ARMING;
            break;
        case Trigger_Both:
            mn = _triggerAdc;
            mx = _triggerAdc;
            return lnDSOAdc::ARMING;
            break;

        default:
            xAssert(0);
            break;
        }
    case lnDSOAdc::ARMING: // ok we are in safe zone, put the trigger
        switch (_triggerMode)
        {
        case Trigger_Rising:
            mn = 0;
            mx = _triggerAdc;
            return lnDSOAdc::ARMED;
            break;
        case Trigger_Falling:
            mn = _triggerAdc;
            mx = 4095;
            return lnDSOAdc::ARMED;
            break;
        case Trigger_Both:
            mn = _triggerAdc;
            mx = _triggerAdc;
            return lnDSOAdc::ARMED;
            break;
        default:
            xAssert(0);
            break;
        }
        break;
    case lnDSOAdc::ARMED:
        return lnDSOAdc::ARMED;
        break;
    default:
        xAssert(0);
        break;
    }
    return lnDSOAdc::ARMED;
}
/**
 *
 * @param s
 */
void DSOCapture::setTriggerVoltage(const float &s)
{
    _triggerVolt = s;
    // convert volt to ADC value
    _triggerAdc = voltToADCValue(s);
}

/**
 *
 * @param v
 * @return
 */
int DSOCapture::voltToADCValue(float v)
{
    float out = v / DSOInputGain::getMultiplier();
    ;
#warning SET AC/DC mode here
    out += DSOInputGain::getOffset(0); // INDEX_AC1_DC0()
    return (int)out;
}

/**
 *
 * @param mode
 */
void DSOCapture::setTriggerMode(TriggerMode mode)
{
    _triggerMode = mode;
}
/**
 *
 * @return
 */
DSOCapture::TriggerMode DSOCapture::getTriggerMode()
{
    return _triggerMode;
}
/**
 *
 * @return
 */
const char *DSOCapture::getTriggerModeAsText()
{
#define TTT(x, y)                                                                                                      \
    case x:                                                                                                            \
        return y;                                                                                                      \
        break;
    switch (_triggerMode)
    {
        TTT(Trigger_Rising, "Up")    // it is inverted !
        TTT(Trigger_Falling, "Down") // it is inverted !
        TTT(Trigger_Both, "Both")
        TTT(Trigger_Run, "None")
    default:
        xAssert(0);
        break;
    }
    return NULL;
}

/**
 *
 * @param voltRange
 * @return
 */
bool DSOCapture::setVoltageRange(DSOCapture::DSO_VOLTAGE_RANGE voltRange)
{
    DSOCapture::currentVoltageRange = voltRange;
    DSOInputGain::setGainRange(vSettings[DSOCapture::currentVoltageRange].gain);
    return true;
}
/**
 *
 * @return
 */
const char *DSOCapture::getVoltageRangeAsText()
{
    return vSettings[DSOCapture::currentVoltageRange].name;
}
float DSOCapture::getVoltToPix()
{
    return vSettings[DSOCapture::currentVoltageRange].displayGain;
}
/**
 *
 * @return
 */
DSOCapture::DSO_VOLTAGE_RANGE DSOCapture::getVoltageRange()
{
    return (DSOCapture::DSO_VOLTAGE_RANGE)DSOCapture::currentVoltageRange;
}

/**
 *
 * @param timeBase
 */
void DSOCapture::setTimeBase(DSO_TIME_BASE timeBase)
{
    currentTimeBase = timeBase;
    xAssert(timerBases[currentTimeBase].timeBase == timerADC[currentTimeBase].timeBase);
    _adc->setSource(timerBases[currentTimeBase].fq, _pin, timerADC[currentTimeBase].scale,
                    timerADC[currentTimeBase].rate, timerADC[currentTimeBase].overSampling);
    Logger("New timebase=%d : %s, fq=%d\n", (int)timeBase, timerBases[timeBase].name, timerBases[timeBase].fq);
}
/**
 *
 * @return
 */
DSOCapture::DSO_TIME_BASE DSOCapture::getTimeBase()
{
    return currentTimeBase;
}
/**
 *
 * @return
 */
const char *DSOCapture::getTimeBaseAsText()
{
    return timerBases[currentTimeBase].name;
}
/**
 *
 * @param pin
 */

const int cycles[8] = {14, 20, 26, 41, 54, 68, 84, 252};

static int lin2log2(int in)
{
    int out = 0;
    while (in > (1 << (out + 1)) && out < 8)
        out++;
    return out;
}
#define THRESHOLD_TO_PREFER_OVERSAMPLING 84
void DSOCapture::initialize(lnPin pin)
{
    internalAdcBuffer = new uint16_t[DSO_CAPTURE_INTERNAL_BUFFER_SIZE];
    _state = CAPTURE_STOPPED;
    _pin = pin;

    // Use max # of cycles possible
    int adcInputClock = lnPeripherals::getClock(pADC0) / 2;
    int maxTimerFq = lnPeripherals::getClock(pTIMER3) / 4;
    bool hasOverSampling = 0 + 1 * (lnCpuID::vendor() == lnCpuID::LN_MCU_GD32);
    for (int i = 0; i < DSO_NB_TIMESCALE; i++)
    {
        timerADC[i].scale = lnADC_CLOCK_DIV_BY_2;
        int samplingFq = timerBases[i].fq;
        int r = adcInputClock / samplingFq;
        int overSampling = 0;

        if (hasOverSampling && r >= 2 * THRESHOLD_TO_PREFER_OVERSAMPLING)
        {
            overSampling = lin2log2(r / THRESHOLD_TO_PREFER_OVERSAMPLING);
            // make sure we done exceed max Timer FQ
            while (((samplingFq << overSampling) > maxTimerFq) && overSampling)
                overSampling--;
            r >>= overSampling;
        }
        timerADC[i].overSampling = overSampling;
        Logger("%d: R=%d overSampling=%d \n", i, r, overSampling);
        // Lookup up the biggest cycle that fits the frequency we need
        // the higher it is, the more accurate the ADC is (?)
        for (int c = 7; c >= 0; c--)
        {
            if (r > cycles[c])
            {
                timerADC[i].rate = (lnADC_CYCLES)c;
                break;
            }
        }
    }
    _adc = new lnDSOAdc(0, TIMER_ADC_ID, TIMER_ADC_CHANNEL); // timer 3 channel 3
    _adc->setCallbacks(&adcCaptureCb);
}
/**
 *
 * @param n
 */
static void captureDone(int n, bool med, int seg)
{

    DSOCapture::captureDone(n, med, seg);
}
void DSOCapture::setCb(captureCb *cb)
{
    _cb = cb;
}
/**
 *
 */
void DSOCapture::captureDone(int nb, bool med, int seg)
{
    xAssert(_cb);
    _state = CAPTURE_DONE;
    _med = med;
    _triggerLocation = seg;
    _cb(); // Data are in the internal buffer, warn client
}
/**
 *
 * @param nb
 * @return
 */
bool DSOCapture::startCapture(int nb)
{
    _nb = nb;
    switch (_state)
    {
    case CAPTURE_RUNNING:
    case CAPTURE_DONE:
        _adc->stopCapture();
        break;
    case CAPTURE_STOPPED:
        break;
    default:
        break;
    }
    _adc->setCb(captureDone);
    _state = CAPTURE_RUNNING;
    xAssert(nb <= DSO_CAPTURE_INTERNAL_BUFFER_SIZE / 2);
    switch (_triggerMode)
    {
    case Trigger_Rising:
    case Trigger_Falling:
    case Trigger_Both:
        return _adc->startTriggeredDma(DSO_CAPTURE_INTERNAL_BUFFER_SIZE, internalAdcBuffer);
        break;
    case Trigger_Run:
        return _adc->startDmaTransfer(nb, internalAdcBuffer); // no need to get the full buffer
        break;
    }
    xAssert(0);
    return false;
}
/**
 *
 */
void DSOCapture::stopCapture()
{
    switch (_state)
    {
    case CAPTURE_RUNNING:
    case CAPTURE_DONE:
        _adc->stopCapture();
        _state = CAPTURE_STOPPED;
        break;
    default:
        break;
    }
}

/**
 *
 * @param nb
 * @param f
 * @return
 */
/**
 *
 * RISCV First : 11k cycles Optim-> same
 *
 * @return
 */

#define BODY()                                                                                                         \
    if (adc < mmin)                                                                                                    \
        mmin = adc;                                                                                                    \
    if (adc > mmax)                                                                                                    \
        mmax = adc;                                                                                                    \
    int fint = adc - offset;                                                                                           \
    float ff = LN_FROM_INT(fint);                                                                                      \
    ff = LN_FP_MUL(ff, multiplier);                                                                                    \
    f[i] = ff; // now in volt

#define LN_MINMAX()                                                                                                    \
    vMin = (float)(mmin - offset) * multiplier;                                                                        \
    vMax = (float)(mmax - offset) * multiplier;

extern uint32_t lnGetCycle32();
bool DSOCapture::getData(int &nb, float *f, float &vMin, float &vMax)
{
    int mmin = 4095;
    int mmax = 0;

    _adc->endCapture();

    if (_triggerMode != Trigger_Run)
    {
        return getDataTriggered(nb, f, vMin, vMax);
    }
    nb = _nb;
    int offset = DSOInputGain::getOffset(_couplingModeIsAC);
    float multiplier = DSOInputGain::getMultiplier();

    // int before=lnGetCycle32();
    for (int i = 0; i < _nb; i++)
    {
        int adc = (int)internalAdcBuffer[(i)];
        BODY()
    }
    // int after=lnGetCycle32();    Logger("Conv 2 volt =%d\n",after-before);

    LN_MINMAX();
    return true;
}
/**
 * \fn getMaxVoltageValue
 * \brief return the max absolute voltage for the current selected voltage range
 * It the actual value is higher than max it means either ADC saturates or we can't display it
 * @return
 */
float DSOCapture::getMaxVoltageValue()
{
    // we want to have less than 80% pixels= 100 (i.e. half screen), else it means saturation

    float gain = vSettings[currentVoltageRange].displayGain;
    float v = 116. / gain;
    return v;
}
/**
 *
 * @return
 */
float DSOCapture::getMinVoltageValue()
{
    // Same but for 2 blocks i.e. 2*24=48 pixel

    float gain = vSettings[currentVoltageRange].displayGain;
    float v = 44. / gain;
    return v;
}
/**
 *
 * @param timeBase
 * @return
 */
int DSOCapture::timeBaseToFrequency(DSOCapture::DSO_TIME_BASE timeBase)
{
#define CASE(x, y)                                                                                                     \
    case DSO_TIME_BASE_##x:                                                                                            \
        return y;                                                                                                      \
        break;

    switch (timeBase)
    {
        // CASE(5US,   500*1000)
        CASE(10US, 100 * 1000)
        CASE(20US, 40 * 1000)
        CASE(50US, 20 * 1000)
        CASE(100US, 10 * 1000)
        CASE(200US, 5 * 1000)
        CASE(500US, 2 * 1000)
        CASE(1MS, 1000)
        CASE(2MS, 500)
        CASE(5MS, 200)
        CASE(10MS, 100)
        CASE(20MS, 50)
        CASE(50MS, 20)
#if 0        
        CASE(100MS, 10) 
        CASE(200MS, 5) 
        CASE(500MS, 2) 
        CASE(1S,    1)
#endif
    }
    xAssert(0);
    return 0;
}

/**
 *
 * @param nb
 * @param f
 * @return
 */
bool DSOCapture::getDataTriggered(int &nb, float *f, float &vMin, float &vMax)
{
    nb = _nb;
    int offset = DSOInputGain::getOffset(_couplingModeIsAC);
    float multiplier = DSOInputGain::getMultiplier();
    int mmin = 4095;
    int mmax = 0;

    // We know the trigger location, rewind a bit so we have the trigger located at the center
    int back = (_triggerLocation - _nb / 2 + DSO_CAPTURE_INTERNAL_BUFFER_SIZE) % DSO_CAPTURE_INTERNAL_BUFFER_SIZE;
    int endPos = (back + _nb) % DSO_CAPTURE_INTERNAL_BUFFER_SIZE;

    debug("TriggerLocation : %d\n", _triggerLocation);

    // does not wrap
    if (endPos > back)
    {
        debug("copy from %d to %d\n", back, back + _nb);
        uint16_t *data16 = internalAdcBuffer + back;
        for (int i = 0; i < _nb; i++)
        {
            int adc = (int)*data16++;
            BODY()
        }
    }
    else
    {
        // till the end of buffer...
        int left = DSO_CAPTURE_INTERNAL_BUFFER_SIZE - back;
        uint16_t *data16 = internalAdcBuffer + back;
        debug("copy 1 : from %d to %d\n", back, back + left);
        for (int i = 0; i < left; i++)
        {
            int adc = (int)*data16++;
            BODY()
        }
        // wrap
        data16 = internalAdcBuffer;
        debug("copy 2 : from %d to %d\n", 0, _nb - left);
        for (int i = left; i < _nb; i++)
        {
            int adc = (int)*data16++;
            BODY()
        }
    }
    // update vMin/vMax
    LN_MINMAX();
    return true;
}
// EOF
