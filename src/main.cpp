#include "lnArduino.h"
#include "gd32_8bits.h"
#include "lnStopWatch.h"
#include "assets/gfx/dso150nb_compressed.h"
#include "dso_gfx.h"
#include "dso_menuEngine.h"
#include "dso_display.h"
#include "dso_test_signal.h"
#include "pinConfiguration.h"
#include "dso_control.h"
#include "DSO_portBArbitrer.h"
#include "dso_capture_stub.h"
#include "dso_calibrate.h"
#include "gd32/nvm_gd32.h"
#include "dso_adc_gain.h"

extern void  menuManagement(void);
extern const GFXfont *smallFont();
extern const GFXfont *mediumFont();
extern const GFXfont *bigFont();
extern void testFunc();
extern void testFunc2();


DSOControl          *control;
DSO_portArbitrer    *arbitrer;
DSO_testSignal      *testSignal;
demoCapture         *capture;
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
    capture=new demoCapture(PA0);
    
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
    
    ili->fillScreen(GREEN);
    ili->setFontFamily(smallFont(),mediumFont(),bigFont());
    ili->setFontSize(ili9341::SmallFont);
    ili->setTextColor(GREEN,BLACK);
    

    
    DSO_GFX::init(ili);
    DSODisplay::init(ili);
    
    
    
    testFunc2();
    Logger("Loading calibration data\n");
    if(!DSOCalibrate::loadCalibrationData())
        DSOCalibrate::zeroCalibrate();
    
    DSOInputGain::readCalibrationValue();
    
    mainLoop();       

}


uint16_t directADC2Read(int pin)
{
    return 0;
}
 void useAdc2(bool use)
 {
     
 }