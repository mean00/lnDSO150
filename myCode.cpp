/***************************************************
 STM32 duino based firmware for DSO SHELL/150
 *  * GPL v2
 * (c) mean 2019 fixounet@free.fr
 ****************************************************/

#include "dso_global.h"
#include "Fonts/waree9.h"
#include "Fonts/waree12.h"
#include "dso_test_signal.h"
#include "dso_eeprom.h"
#include "dso_adc_gain.h"
#include "cpuID.h"
#include "pinConfiguration.h"
static void MainTask( void *a );
void splash(void);
//--
Adafruit_TFTLCD_8bit_STM32 *tft;
testSignal *myTestSignal;
DSOControl *controlButtons;
DSOADC    *adc;
uint16_t displayIdentifier=0;


// Globals
uint16_t calibrationHash=0;

//
// Test functions
//
extern void testTestSignal();
extern void testButtons();
extern void testAdc();
extern void testAdc2();
extern void testAdc3();
extern void testDisplay();
extern void testCalibrate(void);
extern void testI2c( void);
extern void testCapture(void);
extern void testTrigger(void);
extern void testAdcWatchdog(void);
extern void testButtonCoupling(void);
extern void mainDSOUI(void);
/**
 * 
 */

#ifdef __cplusplus
extern "C" {
#endif
void vApplicationDaemonTaskStartupHook(void);
#ifdef __cplusplus
}
#endif


void mySetup() 
{
    
    afio_cfg_debug_ports( AFIO_DEBUG_SW_ONLY); // Unlock PB3 & PB4
#ifdef     USE_RXTX_PIN_FOR_ROTARY // Use Serial only if we have USB
    Serial.begin(115200);
    Serial.println("Init"); 
#endif
    
    
     // Ok let's go, switch to FreeRTOS
    xTaskCreate( MainTask, "MainTask", 250, NULL, DSO_MAIN_TASK_PRIORITY, NULL );
   
    vTaskStartScheduler();      
}
/**
 * 
 */
void vApplicationDaemonTaskStartupHook()
{
}

/**
 * 
 * @param a
 */
void MainTask( void *a )
{    
    cpuID::identify();
    displayIdentifier = tft->readID();
    if(!displayIdentifier) displayIdentifier=0x7789;
    tft=Adafruit_TFTLCD_8bit_STM32::spawn(displayIdentifier);   
    if(!tft)
    {
        while(1) {};
    }
    interrupts();
    tft->begin();
    tft->setRotation(1);
    tft->setFontFamily(&Waree9pt7b, &Waree9pt7b, &Waree12pt7b);  
    tft->fillScreen(BLACK);
    
    splash();
    delay(500);
    myTestSignal=new testSignal(  PIN_TEST_SIGNAL,PIN_TEST_SIGNAL_AMP, 3,TIMER_CH2);
    myTestSignal->setAmplitude(true);
        
    myTestSignal->setFrequency(100000); // 1Khz
    
    controlButtons=new DSOControl ;
    
    controlButtons->setup();
    
    adc=new DSOADC(DSO_INPUT_PIN);
    
    tft->fillScreen(BLACK);
    
    
    tft->setTextSize(3);
   
    //DSOCalibrate::calibrate();
    
    //xAssert(0);
   // testTestSignal();
    //testButtonCoupling();
    //testButtons();   
    if(!DSOEeprom::read())
    {  
        adc->setupADCs ();        
        DSOCalibrate::zeroCalibrate();         
    }
    DSOEeprom::readFineVoltage();
   
    
    //testAdc();   
    //testAdc2();   //fast
    //  testAdc3();   // slow
    //testDisplay();
    // testCalibrate();
    
    DSOInputGain::readCalibrationValue(); // re-read calibration value
    adc->setupADCs ();
    
    // --- TEST  ---
    //DSOCalibrate::voltageCalibrate();
    // --- TEST  ---
    
    DSOCapture::initialize();

    //testI2c();
    mainDSOUI();
    testTrigger();
    testCapture();
    //testAdcWatchdog();
    
    while(1)
    {};
}

void dummyForwardReference()
{
   testTestSignal();
   testButtons();   
   testAdc();   
   testAdc2();   
   testAdc3();  
   testDisplay();
   testCalibrate();
   testI2c();
   testCapture();
   testTrigger();
   testAdcWatchdog();
   mainDSOUI();
}

//-

