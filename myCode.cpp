/***************************************************
 STM32 duino based firmware for DSO SHELL/150
 *  * GPL v2
 * (c) mean 2019 fixounet@free.fr
 ****************************************************/

#include "dso_global.h"
#include "Fonts/waree9.h"
#include "Fonts/waree12.h"
#include "testSignal.h"
#include "HardwareSerial.h"
#include "dso_eeprom.h"
#include "DSO_config.h"
static void MainTask( void *a );
void splash(void);
//--
Adafruit_TFTLCD_8bit_STM32 *tft;
testSignal *myTestSignal;
DSOControl *controlButtons;
DSOADC    *adc;
static uint16_t identifier=0;


// Globals
uint16_t calibrationHash=0;
uint16_t calibrationDC[16];
uint16_t calibrationAC[16];


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
    Serial.begin(115200);
    Serial.println("Init"); 
    
    identifier = tft->readID();
    if(!identifier) identifier=0x7789;
    tft=Adafruit_TFTLCD_8bit_STM32::spawn(0x7789);   
    if(!tft)
    {
        while(1) {};
    }
    tft->begin();
    tft->setRotation(1);
    tft->setFontFamily(&Waree9pt7b, &Waree9pt7b, &Waree12pt7b);  
    tft->fillScreen(BLACK);
    
    splash();
    delay(500);
    myTestSignal=new testSignal(  PA7,PB12, 3,TIMER_CH2);
    myTestSignal->setFrequency(1000); // 1Khz
    myTestSignal->setAmplitute(true);
    
    controlButtons=new DSOControl ;
    
    controlButtons->setup();
    
    adc=new DSOADC;
    
    tft->fillScreen(BLACK);
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
void splash(void)
{
        tft->fillScreen(BLACK);   
        tft->setCursor(45, 10);
        tft->setTextColor(WHITE,BLACK);
        tft->setFontSize(Adafruit_TFTLCD_8bit_STM32::SmallFont);
        tft->myDrawString("    DSO-STM32duino");        
}


/**
 * 
 * @param a
 */
void MainTask( void *a )
{
    interrupts();
    tft->setTextSize(3);
  
  
    
    if(!DSOEeprom::read())
    {
         DSOCalibrate::calibrate();
         DSOEeprom::write();         
    }
   
    adc->readCalibrationValue(); // re-read calibration value
    DSOCapture::initialize();
   // testTestSignal();
   //  testButtons();   
      //testAdc();   
    //testAdc2();   //fast
   //  testAdc3();   // slow
    //testDisplay();
   // testCalibrate();
    //testI2c();
    //testCapture();
    testAdcWatchdog();
    //testTrigger();
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
}

//-

