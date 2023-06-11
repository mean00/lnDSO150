/***************************************************
 * Basic zero calibration UI
 * Switch to all input gain source and mesure the ADC output
 * It will be used later to convert ADC => real voltage
 * (c) mean 2021 fixounet@free.fr
 ****************************************************/

#include "dso_calibrate.h"
#include "DSO_nvmId.h"
#include "dso_adc_gain.h"
#include "dso_capture_input.h"
#include "dso_control.h"
#include "dso_gfx.h"
#include "gd32/nvm_gd32.h"
#include "lnADC.h"
#include "lnArduino.h"

static void printCalibrationTemplate(const char *st1, const char *st2);
static void doCalibrate(uint16_t *array, int color, const char *txt, DSOControl::DSOCoupling target);

//
lnTimingAdc *calAdc = NULL;
extern DSOControl *control;
extern lnNvm *nvm;

/**
 *
 */
static void waitOk()
{
    while (1)
    {
        int evt = control->getQButtonEvent();
        control->getRotaryValue(); // purge
        if (evt == DSO_EVENT_Q(DSOControl::DSO_BUTTON_OK, EVENT_SHORT_PRESS))
            return;
        xDelay(10);
    }
}
/**
 *
 * @return
 */
bool DSOCalibrate::loadCalibrationData()
{
    uint16_t data[DSO_NB_GAIN_RANGES + 1];

    if (!nvm->read(NVM_CALIBRATION_AC, DSO_NB_GAIN_RANGES * 2 + 2, (uint8_t *)data))
    {
        Logger("Cannot load AC calibration\n");
        return false;
    }
    if (data[0] != NVM_CALIBRATION_VERSION)
    {
        Logger("AC:Wrong version %x vs expected %x\n", data[0], NVM_CALIBRATION_VERSION);
        return false;
    }
    for (int i = 0; i < DSO_NB_GAIN_RANGES; i++)
        calibrationAC[i] = data[i + 1];

    if (!nvm->read(NVM_CALIBRATION_DC, DSO_NB_GAIN_RANGES * 2 + 2, (uint8_t *)data))
    {
        Logger("Cannot load DC calibration\n");
        return false;
    }
    if (data[0] != NVM_CALIBRATION_VERSION)
    {
        Logger("DC:Wrong version %x vs expected %x\n", data[0], NVM_CALIBRATION_VERSION);
        return false;
    }
    for (int i = 0; i < DSO_NB_GAIN_RANGES; i++)
        calibrationDC[i] = data[i + 1];
    return true;
}

/**
 *
 * @return
 */
bool DSOCalibrate::zeroCalibrate()
{
    // Catch control callback
    DSOControl::ControlEventCb *oldCb = control->getCb();
    bool r = zeroCalibrate_();
    control->changeCb(oldCb);
    return r;
}
/**
 *
 * @return
 */
bool saveCalibrationData()
{
    uint16_t data[DSO_NB_GAIN_RANGES + 1];

    data[0] = NVM_CALIBRATION_VERSION;
    for (int i = 0; i < DSO_NB_GAIN_RANGES; i++)
        data[i + 1] = calibrationAC[i];

    if (!nvm->write(NVM_CALIBRATION_AC, DSO_NB_GAIN_RANGES * 2 + 2, (uint8_t *)data))
    {
        return false;
    }
    for (int i = 0; i < DSO_NB_GAIN_RANGES; i++)
        data[i + 1] = calibrationDC[i];

    if (!nvm->write(NVM_CALIBRATION_DC, DSO_NB_GAIN_RANGES * 2 + 2, (uint8_t *)data))
    {
        return false;
    }
    return true;
}
/**
 *
 * @return
 */
bool DSOCalibrate::zeroCalibrate_()
{
    DSO_GFX::setBigFont(false);
    DSO_GFX::setTextColor(WHITE, BLACK);

    printCalibrationTemplate("Connect the probe to", "ground");
    waitOk();

    calAdc = new lnTimingAdc(0);
    lnPin pin = PA0;
    calAdc->setSource(3, 3, 1000, 1, &pin);

    doCalibrate(calibrationDC, YELLOW, "", DSOControl::DSO_COUPLING_DC);
    doCalibrate(calibrationAC, GREEN, "", DSOControl::DSO_COUPLING_AC);
    const char *msg = "Restart the unit.";
    DSO_GFX::center("@- saving -@", 8);
    if (!saveCalibrationData())
    {
        msg = "Failed to save cal";
    }
    DSO_GFX::clear(0);
    DSO_GFX::printxy(5, 4, msg);
    delete calAdc;
    calAdc = NULL;
    while (1)
    {
    };
    return true;
}
/**
 */
void printCalibrationTemplate(const char *st1, const char *st2)
{
    DSO_GFX::newPage("CALIBRATION");
    DSO_GFX::center(st1, 4);
    DSO_GFX::center(st2, 5);
    DSO_GFX::bottomLine("and press @OK@");
}

/**
 *
 * @param cpl
 */
static void printCoupling(DSOControl::DSOCoupling cpl)
{
    static const char *coupling[3] = {"currently : GND  ", "currently : DC   ", "currently : AC   "};
    DSO_GFX::center(coupling[cpl], 5);
}
/**
 *
 * @param target
 */
static void waitForCoupling(DSOControl::DSOCoupling target)
{
    DSOControl::DSOCoupling cpl = (DSOControl::DSOCoupling)-1;
    const char *st = "Set input to DC";
    if (target == DSOControl::DSO_COUPLING_AC)
        st = "Set input to AC";
    DSO_GFX::center(st, 4);
    while (1)
    {
        DSOControl::DSOCoupling newcpl = control->getCouplingState();
        if (newcpl == target)
            DSO_GFX::setTextColor(GREEN, BLACK);
        else
            DSO_GFX::setTextColor(RED, BLACK);

        if (cpl != newcpl)
        {
            printCoupling(newcpl);
            cpl = newcpl;
        }
        control->getRotaryValue();
        if (cpl == target)
        {
            if (control->getQButtonEvent() == DSO_EVENT_Q(DSOControl::DSO_BUTTON_OK, EVENT_SHORT_PRESS))
            {
                return;
            }
        }
    }
}
/**
 *
 * @param color
 * @param txt
 * @param target
 */
void header(int color, const char *txt, DSOControl::DSOCoupling target)
{
    printCalibrationTemplate(txt, "");
    waitForCoupling(target);
    DSO_GFX::center("@- processing -@", 6);
    return;
}
#define NB_SAMPLES 64
/**
 *
 * @return
 */
static int averageADCRead()
{
#define NB_POINTS 16
    uint16_t samples[NB_POINTS];
    calAdc->multiRead(NB_POINTS, samples);
    int sum = 0;
    for (int i = 0; i < NB_POINTS; i++)
    {
        sum += samples[i];
    }
    sum = (sum + (NB_POINTS / 2 - 1)) / NB_POINTS;
    return sum;
}

/**
 *
 * @param array
 * @param color
 * @param txt
 * @param target
 */
void doCalibrate(uint16_t *array, int color, const char *txt, DSOControl::DSOCoupling target)
{
    header(color, txt, target);
    for (int range = DSOInputGain::MAX_VOLTAGE_GND; range <= DSOInputGain::MAX_VOLTAGE_20V; range++)
    {
        DSOInputGain::setGainRange((DSOInputGain::InputGainRange)range);
        xDelay(10);
        array[range] = averageADCRead();
        Logger("Range : %d val=%d\n", range, array[range]);
    }
}

/**
 *
 * @return
 */
bool DSOCalibrate::decalibrate()
{
    uint8_t empty[2] = {0xff, 0xff};
    nvm->write(NVM_CALIBRATION_DC, 2, empty);
    nvm->write(NVM_CALIBRATION_AC, 2, empty);
    return true;
}
//

class VccAdc : public lnBaseAdc
{
  public:
    VccAdc(int instance) : lnBaseAdc(instance)
    {
    }
    virtual ~VccAdc()
    {
    }
    //            int      getVref(); // direct value
    //    static  float    getVcc();  // Vcc value in mv
    void readVcc()
    {
        lnBaseAdc::readVcc();
    }
    void setup()
    {
        lnBaseAdc::setup();
    }
};

/**
 *
 * @return
 */
bool DSOCalibrate::vccAdcMenu()
{
    DSO_GFX::setBigFont(false);
    DSO_GFX::setTextColor(WHITE, BLACK);

    DSO_GFX::newPage("VCC");

    VccAdc *adc = new VccAdc(0);
    adc->setup();

    char buffer[80];

    while (1)
    {
        adc->readVcc();
        int raw = adc->getVref();
        float volt = adc->getVcc() / 1000.;
        sprintf(buffer, "RAW:%d_____", raw);
        DSO_GFX::printxy(5, 2, buffer);
        sprintf(buffer, "VOLT:%2.2f_____", volt);
        DSO_GFX::printxy(5, 4, buffer);
        lnDelayMs(1000);
    }

    delete adc;
    adc = NULL;
    while (1)
    {
    };
    return true;
}

// EOF