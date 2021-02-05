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
#include "helpers/helper_pwm.h"
#include "dso_debug.h"
static void MainTask( void *a );
void splash(void);
//--
Adafruit_TFTLCD_8bit_STM32 *tft;
testSignal *myTestSignal;
DSOControl *controlButtons;
DSOADC    *adc;
uint16_t displayIdentifier=0;
xMutex   *adc2Lock;

// Globals
uint16_t calibrationHash=0;

//
// Test functions
//
#include "testFunc/testFunc.h"

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
#if 0
    __IO uint32 *mapr = &AFIO_BASE->MAPR;
    uint32_t r=*mapr;
    r=r & ~AFIO_MAPR_SWJ_CFG;
    r|=AFIO_DEBUG_SW_ONLY;
    *mapr=r;
#endif
  Serial.end();
  Serial1.begin(115200);  //Wire.begin();
  Serial.end();
  Serial1.begin(115200);  
    
  Logger("Init"); 

  cpuID::identify(); // enable FPU ASAP
    
  // Ok let's go, switch to FreeRTOS
  xTaskCreate( MainTask, "MainTask", 350, NULL, DSO_MAIN_TASK_PRIORITY, NULL );   
  vTaskStartScheduler();      
}
/**
 * 
 */
void vApplicationDaemonTaskStartupHook()
{
}
int adcLockCount=0;
void useAdc2(bool use)
{
    if(use)
    {             
        adc2Lock->lock();        
        xAssert(!adcLockCount);        
        adcLockCount++;
        
    }
    else
    {
        xAssert(adcLockCount==1);        
        adcLockCount--;
        adc2Lock->unlock();
    }
}
/**
 * 
 * @param a
 */
void MainTask( void *a )
{    
    
    displayIdentifier = tft->readID();
    if(!displayIdentifier) displayIdentifier=0x7789;
    tft=Adafruit_TFTLCD_8bit_STM32::spawn(displayIdentifier);   
    if(!tft)
    {
        xAssert(0);
    }
    interrupts();
    tft->begin();
    tft->setRotation(1);
    tft->setFontFamily(&Waree9pt7b, &Waree9pt7b, &Waree12pt7b);  
    tft->fillScreen(BLACK);
    
    splash();
    
    adc2Lock=new xMutex;
    
    delay(500);
    myTestSignal=new testSignal(  PIN_TEST_SIGNAL,PIN_TEST_SIGNAL_AMP);
    myTestSignal->setAmplitude(true);
        
    myTestSignal->setFrequency(1000); // 1Khz
    
    controlButtons=new DSOControl ;
    
    controlButtons->setup();
    
    adc=new DSOADC(DSO_INPUT_PIN);
    
    tft->fillScreen(BLACK);
    
    
    tft->setTextSize(3);
   
    //DSOCalibrate::calibrate();
    // Start ADC timer timebase at 8 Mhz
    // total period = 0.125 us
    setTimerFrequency(&Timer2,2, 62, 63); 
    //xAssert(0);
    adc->setupADCs ();       


 // Do a dummy capture to make sure everything is fine
    
    adc->setADCPin(PA0);
    adc->setupDmaSampling();
    adc->prepareDMASampling(ADC_SMPR_239_5,DSOADC::ADC_PRESCALER_8);    
    adc->stopDmaCapture();



    //testCalibrate();
   // testTestSignal();
    //testButtonCoupling();
    //testButtons();   
    //testDualADC();
    //testPigOsCope();
    //
    //
    if(!DSOEeprom::read())
    {  
        DSOEeprom::format();
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
   mainDSOUI();
}

 float testFpu(float f, float g)
 {
     return f*g-1.1;
 }
//-

