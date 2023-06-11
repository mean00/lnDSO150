/***************************************************
 STM32 duino based firmware for DSO SHELL/150
 *  * GPL v2
 * (c) mean 2019 fixounet@free.fr
 ****************************************************/
#include "dso_calibrate.h"
#include "dso_control.h"
#include "dso_gfx.h"
#include "dso_menuEngine.h"
#include "dso_test_signal.h"
#include "dso_version.h"
#include "lnArduino.h"
#include "lnCpuID.h"

extern DSO_testSignal *testSignal;
static int currentFQ;
static int largeAmplitude;
static int invertedRotary = 0;

#define MAKEFQITEM(name, val) static const MenuListItem name = {&currentFQ, val};

MAKEFQITEM(fq100, 100)
MAKEFQITEM(fq1k, 1000)
MAKEFQITEM(fq10k, 10 * 1000)
MAKEFQITEM(fq100k, 100 * 1000)
MAKEFQITEM(fq50k, 50 * 1000)

#define FQ_MENU(x, y) {MenuItem::MENU_INDEX, x, (void *)&(fq##y)},
const MenuItem fqMenu[] = {{MenuItem::MENU_TITLE, "Frequency", NULL},
                           FQ_MENU("100 Hz", 100) FQ_MENU("1 kHz", 1k) FQ_MENU("10 kHz", 10k)
                               FQ_MENU("50 kHz", 50k){MenuItem::MENU_END, NULL, NULL}};

#define MAKEAMPLITEM(name, val) static const MenuListItem name = {&largeAmplitude, val};
MAKEAMPLITEM(ampl3v, 1)
MAKEAMPLITEM(ampl0v, 0)

#define RANGE_MENU(x, y) {MenuItem::MENU_INDEX, x, (void *)(&ampl##y)},
//--
const MenuItem amplitudeMenu[] = {{MenuItem::MENU_TITLE, "Amplitude", NULL},
                                  RANGE_MENU("3.3v", 3v) RANGE_MENU("100mv", 0v){MenuItem::MENU_END, NULL, NULL}};
//--

const MenuItem signalMenu[] = {{MenuItem::MENU_TITLE, "Test Signal", NULL},
                               {MenuItem::MENU_SUBMENU, "Amplitude", (const void *)&amplitudeMenu},
                               {MenuItem::MENU_SUBMENU, "Frequency", (const void *)&fqMenu},
                               {MenuItem::MENU_CALL, "VCC ADC", (const void *)DSOCalibrate::vccAdcMenu},
                               {MenuItem::MENU_END, NULL, NULL}};

#define MAKEINVERTITEM(name, val) static const MenuListItem name = {&invertedRotary, val};
MAKEINVERTITEM(rotDirect, 0)
MAKEINVERTITEM(rotInverted, 1)

#define INVERT_MENU(x, y) {MenuItem::MENU_INDEX, x, (void *)(&rot##y)},

const MenuItem controlMenu[] = {{MenuItem::MENU_TITLE, "Control", NULL},
                                INVERT_MENU("Clockwise", Direct)
                                    INVERT_MENU("CounterClock", Inverted){MenuItem::MENU_END, NULL, NULL}};

//--
const MenuItem calibrationMenu[] = {
    {MenuItem::MENU_TITLE, "Calibration", NULL},
    {MenuItem::MENU_CALL, "Basic Calibrate", (const void *)DSOCalibrate::zeroCalibrate},
    //{MenuItem::MENU_CALL, "Fine Calibrate",(const void *)DSOCalibrate::voltageCalibrate},
    {MenuItem::MENU_CALL, "Wipe Calibration", (const void *)DSOCalibrate::decalibrate},
    {MenuItem::MENU_END, NULL, NULL}};
/**
 *
 */
#define MKSTRING(x) #x
static const char *getSwVersion()
{
    static char versionString[20];

    sprintf(versionString, "%d.%d %s", DSO_VERSION_MAJOR, DSO_VERSION_MINOR, LN_GIT_REV);
    return versionString;
}
extern const char *getScreenID();

//--
const MenuItem infoMenu[] = {{MenuItem::MENU_TITLE, "Information", NULL},
                             {MenuItem::MENU_TEXT, "MC", (const void *)lnCpuID::idAsString},
                             {MenuItem::MENU_TEXT, "SW", (const void *)getSwVersion},
                             {MenuItem::MENU_TEXT, "SC", (const void *)getScreenID},
                             {MenuItem::MENU_END, NULL, NULL}};

//--
const MenuItem topMenu[] = {{MenuItem::MENU_TITLE, "Main Menu", NULL},
                            {MenuItem::MENU_SUBMENU, "Test signal", (const void *)&signalMenu},
                            //{MenuItem::MENU_CALL, "Button Test",(const void *)buttonTest},
                            {MenuItem::MENU_SUBMENU, "Calibration", (const void *)&calibrationMenu},
                            {MenuItem::MENU_SUBMENU, "Control", (const void *)&controlMenu},
                            {MenuItem::MENU_SUBMENU, "Info", (const void *)&infoMenu},
                            {MenuItem::MENU_END, NULL, NULL}};

/**
 *
 * @param control
 */
void menuManagement(DSOControl *control)
{
    DSO_GFX::clear(BLACK);
    const MenuItem *tem = topMenu;

    currentFQ = testSignal->getFrequency();
    largeAmplitude = testSignal->getAmplitude();
    invertedRotary = control->inverted();
    MenuManager man(control, tem);
    man.run();

    testSignal->setFrequency(currentFQ);
    testSignal->setAmplitude(largeAmplitude);
    control->invert(invertedRotary);
}
// EOF
