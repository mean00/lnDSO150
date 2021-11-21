#include "lnArduino.h"
#include "gd32_8bits.h"
#include "lnStopWatch.h"
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
#include "dso_capture.h"
#include "lnCpuID.h"

extern void  menuManagement(void);
extern const GFXfont *smallFont();
extern const GFXfont *mediumFont();
extern const GFXfont *bigFont();
extern void testFunc();
extern void testFunc2();



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


void mainLoop_bounce(void *a)
{
    mainLoop();
}

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
    
    
    DSO_GFX::init(ili);
    DSODisplay::init(ili);
    DSODisplay::drawSplash();
    
    testFunc2();
   
    xTaskCreate(mainLoop_bounce, "mainLoop",1000,NULL,4,NULL);
    while(1)
    {
        xDelay(1000);
    }
}
//