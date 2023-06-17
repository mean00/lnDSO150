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
static void waitForCoupling(DSOControl::DSOCoupling target);
static int averageADCRead();
//
lnTimingAdc *calAdc = NULL;
extern DSOControl *control;
extern lnNvm *nvm;
extern float vref_adc_mul;

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

float vref_adc_mul = 1.0;
/**
 *
 * @return
 */
bool DSOCalibrate::loadCalibrationData()
{
    uint16_t data[DSO_NB_GAIN_RANGES + 1];

    if (!nvm->read(NVM_ADC_VREF_MULTIPLIER, sizeof(float), (uint8_t *)&vref_adc_mul))
    {
        vref_adc_mul = 1.; // 1.0086
    }

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
    uint16_t *ac = DSOInputGain::getCalibrationTable(1);
    for (int i = 0; i < DSO_NB_GAIN_RANGES; i++)
        ac[i] = data[i + 1];

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
    uint16_t *dc = DSOInputGain::getCalibrationTable(0);
    for (int i = 0; i < DSO_NB_GAIN_RANGES; i++)
        dc[i] = data[i + 1];
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
bool DSOCalibrate::fiveVoltCalibrate()
{
    //

    DSOControl::ControlEventCb *oldCb = control->getCb();
    control->changeCb(NULL);
    // Force DC mode
    DSO_GFX::newPage("5 V CALIBRATION");
    waitForCoupling(DSOControl::DSO_COUPLING_DC);

    // Catch control callback
    bool r = true;
    r = fiveVoltCalibrate_();
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
    uint16_t *ac = DSOInputGain::getCalibrationTable(1);
    for (int i = 0; i < DSO_NB_GAIN_RANGES; i++)
        data[i + 1] = ac[i];

    if (!nvm->write(NVM_CALIBRATION_AC, DSO_NB_GAIN_RANGES * 2 + 2, (uint8_t *)data))
    {
        return false;
    }
    uint16_t *dc = DSOInputGain::getCalibrationTable(1);
    for (int i = 0; i < DSO_NB_GAIN_RANGES; i++)
        data[i + 1] = dc[i];

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

    uint16_t *ac = DSOInputGain::getCalibrationTable(1);
    uint16_t *dc = DSOInputGain::getCalibrationTable(0);

    doCalibrate(dc, YELLOW, "", DSOControl::DSO_COUPLING_DC);
    doCalibrate(ac, GREEN, "", DSOControl::DSO_COUPLING_AC);
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
extern float raw_multipliers[];
bool DSOCalibrate::fiveVoltCalibrate_()
{
    DSOInputGain::InputGainRange range = DSOInputGain::MAX_VOLTAGE_8V;
    DSOInputGain::setGainRange(range);
    DSO_GFX::newPage("5V calibration");
    DSO_GFX::center("Connect to 5v", 2);
    DSO_GFX::printxy(2, 2, "Uncorrected voltage");
    DSO_GFX::bottomLine("@VOLT@=RESET    @OK@=SET");

    calAdc = new lnTimingAdc(0);
    lnPin pin = PA0;
    calAdc->setSource(3, 3, 1000, 1, &pin);

    // Loop
    float fvcc = lnBaseAdc::getVcc();
    float mu = fvcc / (4095. * 1000.);
    int offset = DSOInputGain::getOffset(0);
    mu = mu * raw_multipliers[range];
    char buffer[40];
#define FIVE_WAIT 0
#define FIVE_SET 1
#define FIVE_RESET 2
    int run = FIVE_WAIT;
    float v;
    //--
    while (run == FIVE_WAIT)
    {
        bool valid = false;
        int raw = averageADCRead();
        raw -= offset;
        v = (float)raw * mu;

        sprintf(buffer, "%2.2f_", v);

        if (v >= 3.5 && v <= 6.5)
        {
            DSO_GFX::setTextColor(BLACK, GREEN);
            valid = true;
        }
        else
        {
            DSO_GFX::setTextColor(BLACK, RED);
            valid = false;
        }

        DSO_GFX::printxy(5, 3, buffer);
        switch (control->getQButtonEvent())
        {
        case DSO_EVENT_Q(DSOControl::DSO_BUTTON_OK, EVENT_SHORT_PRESS):
            if (valid)
            {
                run = FIVE_SET;
                continue;
            }
            break;
        case DSO_EVENT_Q(DSOControl::DSO_BUTTON_VOLTAGE, EVENT_SHORT_PRESS):
            run = FIVE_RESET;
            break;
        default:
            break;
        }
        lnDelayMs(20); // yield
    }

    delete calAdc;
    calAdc = NULL;

    switch (run)
    {
    case FIVE_SET:
        vref_adc_mul = 5. / v;
        break;
    case FIVE_RESET:
        vref_adc_mul = 1.0;
        break;
    default:
        xAssert(0);
        break;
    }

    nvm->write(NVM_ADC_VREF_MULTIPLIER, sizeof(vref_adc_mul), (uint8_t *)&vref_adc_mul);
    DSOInputGain::preComputeMultiplier();
    DSOInputGain::postComputeMultiplier();
    DSO_GFX::clear(BLACK);
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
void waitForCoupling(DSOControl::DSOCoupling target)
{
    DSOControl::DSOCoupling cpl = (DSOControl::DSOCoupling)-1;
    const char *st = "Set input to DC";
    if (target == DSOControl::DSO_COUPLING_AC)
        st = "Set input to AC";
    DSO_GFX::center(st, 4);
    DSO_GFX::bottomLine("and press @OK@");

    while (1)
    {
        lnDelayMs(100); // yield
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
int averageADCRead()
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
    uint8_t empty[4] = {0xff, 0xff, 0xff, 0xff};
    nvm->write(NVM_CALIBRATION_DC, 2, empty);
    nvm->write(NVM_CALIBRATION_AC, 2, empty);

    float one = 1.0;
    if (!nvm->write(NVM_ADC_VREF_MULTIPLIER, sizeof(one), (uint8_t *)&one))
    {
        Logger("Failed to rst vref\n");
    }

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
        float volt = (adc->getVcc()) / 1000.;
        float volt_adj = volt * vref_adc_mul;
        sprintf(buffer, "RAW:%d_____", raw);
        DSO_GFX::printxy(5, 1, buffer);
        sprintf(buffer, "VOLT:%2.2f_____", volt);
        DSO_GFX::printxy(5, 3, buffer);
        sprintf(buffer, "ADJ:%2.2f_____", volt_adj);
        DSO_GFX::printxy(5, 5, buffer);

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