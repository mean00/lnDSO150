#include "DSO_portBArbitrer.h"
#include "dso_adc_gain.h"
#include "dso_calibrate.h"
#include "dso_capture.h"
#include "dso_config.h"
#include "dso_control.h"
#include "dso_display.h"
#include "dso_gfx.h"
#include "dso_menuEngine.h"
#include "dso_test_signal.h"
#include "gd32/nvm_gd32.h"
#include "ili_ln8bits.h"
#include "lnArduino.h"
#include "lnCpuID.h"
#include "lnStopWatch.h"
#include "pinConfiguration.h"

extern void menuManagement(void);
extern const GFXfont *smallFont();
extern const GFXfont *mediumFont();
extern const GFXfont *bigFont();
extern void testFunc();
extern void testFunc2();
extern void mainLoop();

extern const uint8_t dso_resetOff[];
extern const uint8_t dso_wakeOn[];

DSOControl *control;
DSO_portArbitrer *arbitrer;
DSO_testSignal *testSignal;
ln8bit9341 *ili;
lnNvm *nvm;
uint32_t chipId;

extern void testFunc();

/**
 *
 */
void setup()
{
    Logger("Setuping up DSO...\n");
    lnMutex *PortBMutex = new lnMutex;

    // arbitrer must be created with screen already set up
    // ili must be first
    ili = new ln8bit9341(240, 320,
                         1,    // port B
                         PC14, // DC/RS
                         PC13, // CS
                         PC15, // Write
                         PA6,  // Read
                         PB9); // LCD RESET
    ili->init(dso_resetOff, dso_wakeOn);
    ili->setRotation(1);

    ili->setFontFamily(smallFont(), mediumFont(), bigFont());
    ili->setFontSize(ili9341::SmallFont);
    ili->setTextColor(GREEN, BLACK);

    DSO_GFX::init(ili);
    DSODisplay::init(ili);

    arbitrer = new DSO_portArbitrer(1, PortBMutex); // arbitrer must be created with screen already set up

    // testFunc();

    DSODisplay::drawSplash();

    control = new DSOControl(NULL); // control must be initialised after ili !
    control->setup();

    testSignal = new DSO_testSignal(PIN_TEST_SIGNAL, PIN_TEST_SIGNAL_AMP);
    testSignal->setFrequency(1 * 1000);
    DSOCapture::initialize(PA0);

    nvm = new lnNvmGd32();
    if (!nvm->begin())
    {
        Logger("Nvm not operational, reformating... \n");
        nvm->format();
    }
}

/**
 *
 */
void loop()
{

    Logger("Starting DSO...\n");
    mainLoop();
}
/**
 *
 */
const char *getScreenID()
{
    uint32_t id = ili->getChipId();
    const char *t = "?????";
    switch (id)
    {
    case 0x9341:
        t = "ILI9341";
        break;
    case 0x7789:
        t = "ST7789";
        break;
    default:
        break;
    }
    return t;
}

//
