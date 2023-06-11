/***************************************************
 STM32 duino based firmware for DSO SHELL/150
 * Try to auto set voltage range & frequency
 * Could be better, but that will do for now
 *  * GPL v2
 * (c) mean 2019 fixounet@free.fr
 ****************************************************/
#include "dso_capture.h"
#include "dso_display.h"
#include "lnArduino.h"
#include "math.h"

static bool autoSetupVoltage(bool setTrigger);
static bool autoSetupFrequency();
static void autoTrigger();

static captureCb *oldCb;
static xBinarySemaphore *sem = NULL;
static float *capture = NULL;

extern void setVoltageOffset(float v);
extern float *captureBuffer;
void setupCb()
{
    sem->give();
}

/**
 *
 */
void autoSetup()
{
    capture = captureBuffer;
    DSOCapture::stopCapture();
    if (!sem)
        sem = new xBinarySemaphore;

    DSODisplay::drawAutoSetup();
    oldCb = DSOCapture::getCb();
    DSOCapture::setCb(setupCb);

    // switch to free running mode

    //   DSOCapture::setTriggerMode(DSOCapture::Trigger_Run);

    DSOCapture::DSO_TIME_BASE timeBase = DSOCapture::DSO_TIME_BASE_1MS;
    DSOCapture::setTimeBase(timeBase);
    setVoltageOffset(0);

    DSOCapture::setTriggerMode(DSOCapture::Trigger_Run);

    // voltage range

    if (!autoSetupVoltage(false))
        goto end; // failed
    DSODisplay::drawAutoSetupStep(1);
    if (!autoSetupFrequency())
        goto end;
    DSODisplay::drawAutoSetupStep(2);
    // redo voltage in case it was wrong the 1st time due to too high/too low fq
    if (!autoSetupVoltage(true))
        goto end; // failed
    DSODisplay::drawAutoSetupStep(3);

    autoTrigger();
end:
    DSOCapture::stopCapture();
    DSOCapture::setCb(oldCb);
    capture = NULL;
    return;
}

/**
 *
 * @return
 */
bool autoSetupVoltage(bool setTrigger)
{
    int voltage = DSOCapture::DSO_VOLTAGE_5V;
    DSOCapture::setVoltageRange((DSOCapture::DSO_VOLTAGE_RANGE)voltage);

    int tries = NB_CAPTURE_VOLTAGE + 2;
    int nb;
    float xMin, xMax;
    while (1) //! clock.elapsed(2000))
    {
        sem->tryTake();
        DSOCapture::startCapture(240);
        sem->take();
        DSOCapture::getData(nb, capture, xMin, xMax);

        xMax = fabs(xMax);
        xMin = fabs(xMin);

        if (tries-- < 0)
            return true; // did not converge ?
        // Are we over the max ?
        if (xMax > DSOCapture::getMaxVoltageValue() && voltage < DSOCapture::DSO_VOLTAGE_MAX) // saturation
        {
            voltage = voltage + 1; // yes, use higher scale
            DSOCapture::setVoltageRange((DSOCapture::DSO_VOLTAGE_RANGE)voltage);
            continue;
        }
        // is it too small ?, if so take a lower (more zoom) range
        if (xMax < DSOCapture::getMinVoltageValue() &&
            voltage > 1) // too small, voltage =0 means ground, we dont want it
        {
            voltage = voltage - 1;
            DSOCapture::setVoltageRange((DSOCapture::DSO_VOLTAGE_RANGE)voltage);
            continue;
        }
        return true;
    }
    return false;
}

/**
 *
 * @return
 */
bool autoSetupFrequency()
{
    int timeBase = (int)DSOCapture::DSO_TIME_MIN;
    DSOCapture::setTimeBase((DSOCapture::DSO_TIME_BASE)timeBase);
    int tries = 20;
    while (1)
    {
        if (timeBase > DSOCapture::DSO_TIME_BASE_MAX)
        {
            break;
        }

        DSOCapture::startCapture(240);
        sem->take();
        // get fq
        int fq = DSOCapture::computeFrequency();
        Logger("timeBase: %d Fq=%d\n", timeBase, fq);
        if (fq)
        {
            if (fq > 30)
            {
                // Try to get fq = 4 square
                int qfq = DSOCapture::timeBaseToFrequency((DSOCapture::DSO_TIME_BASE)timeBase);
                if (fq <= qfq * 4)
                {
                    return true;
                }
            }
        }
        timeBase++;
        if (timeBase > DSOCapture::DSO_TIME_BASE_MAX)
            break;
        DSOCapture::setTimeBase((DSOCapture::DSO_TIME_BASE)timeBase);
    }
    DSOCapture::setTimeBase(DSOCapture::DSO_TIME_BASE_1MS);
    return false;
}
/**
 *
 */
void autoTrigger()
{
    float xMin, xMax;

    sem->tryTake();
    DSOCapture::startCapture(240);
    sem->take();
    int nb;
    DSOCapture::getData(nb, capture, xMin, xMax);

    float trigger = (xMax + xMin) / 2;
    DSOCapture::setTriggerVoltage(trigger);

    if (((xMax - xMin)) > 0.1)
        DSOCapture::setTriggerMode(DSOCapture::Trigger_Rising);
    else
        DSOCapture::setTriggerMode(DSOCapture::Trigger_Run);
}