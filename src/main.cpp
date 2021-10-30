#include "lnArduino.h"
#include "gd32_8bits.h"
#include "lnStopWatch.h"
#include "assets/gfx/generated/splash_decl.h"
#include "dso_gfx.h"
#include "dso_menuEngine.h"
#include "dso_display.h"
#include "dso_test_signal.h"
#include "pinConfiguration.h"
#include "dso_control.h"
#include "DSO_portBArbitrer.h"
#include "dso_calibrate.h"
#include "gd32/nvm_gd32.h"
#include "dso_adc_gain.h"
#include "dso_adc_capture.h"
#include "lnCpuID.h"

extern void  menuManagement(void);
extern const GFXfont *smallFont();
extern const GFXfont *mediumFont();
extern const GFXfont *bigFont();
extern void testFunc();
extern void testFunc2();
extern const uint8_t *getSplash();

static void drawSplash();

DSOControl          *control;
DSO_portArbitrer    *arbitrer;
DSO_testSignal      *testSignal;
ln8bit9341          *ili;
lnNvm               *nvm;

/**
 * 
 */
void setup()
{
    Logger("Setuping up DSO...\n");
    xMutex *PortBMutex=new xMutex;
    arbitrer=new DSO_portArbitrer(1,PortBMutex);
    
    control=new DSOControl(NULL); // control must be initialised after ili !
    control->setup();

    
    testSignal=new DSO_testSignal(PIN_TEST_SIGNAL,PIN_TEST_SIGNAL_AMP);
    testSignal->setFrequency(1*1000);
    DSOCapture::initialize(PA0);
    
    nvm=new lnNvmGd32();
    if(!nvm->begin())
    {
        Logger("Nvm not operational, reformating... \n");
        nvm->format();
    }
}
uint32_t chipId;
extern void mainLoop();

/**
 * 
 */
void loop()
{
    
    Logger("Starting DSO...\n");
    testFunc();
    ili=new ln8bit9341( 240, 320,
                                    1,          // port B
                                    PC14,       // DC/RS
                                    PC13,       // CS
                                    PC15,       // Write
                                    PA6,        // Read
                                    PB9);       // LCD RESET
    ili->init();    
    ili->setRotation(1);
    
    ili->setFontFamily(smallFont(),mediumFont(),bigFont());
    ili->setFontSize(ili9341::SmallFont);
    ili->setTextColor(GREEN,BLACK);
    
    drawSplash();
    
    
    
    DSO_GFX::init(ili);
    DSODisplay::init(ili);
    
    testFunc2();
    Logger("Loading calibration data\n");
    if(!DSOCalibrate::loadCalibrationData())
        DSOCalibrate::zeroCalibrate();
    
    mainLoop();       
}
/**
 * 
 */

#define DSO_VERSION_MAJOR 2
#define DSO_VERSION_MINOR 0

void drawSplash()
{
    ili->fillScreen(BLACK);   
    ili->drawRLEBitmap(splash_width,splash_height,2,2, WHITE,BLACK,getSplash());
    ili->setFontSize(ili9341::SmallFont);
    ili->setTextColor(WHITE,BLACK);
    ili->setCursor(140, 64);
    ili->print("lnDSO150");              
    ili->setCursor(140, 84);
#ifdef USE_RXTX_PIN_FOR_ROTARY        
        ili->print("USB  Version");              
#else
        tft->print("RXTX Version");              
#endif
    char bf[20];
    sprintf(bf,"%d.%02d",DSO_VERSION_MAJOR,DSO_VERSION_MINOR);
    ili->setCursor(140, 64+20*2);        
    ili->print(bf);       
    ili->setCursor(140, 64+20*4);
    ili->print(lnCpuID::idAsString());         
    ili->setCursor(140, 64+20*5);
    sprintf(bf,"%d Mhz",lnCpuID::clockSpeed()/1000000);
    ili->print(bf);         
    xDelay(500);
}

//