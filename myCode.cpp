/***************************************************
 STM32 duino based firmware for DSO SHELL/150
 *  * GPL v2
 * (c) mean 2019 fixounet@free.fr
 ****************************************************/

#include <Wire.h>
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_TFTLCD_8bit_STM32.h"
//#include "Fonts/digitLcd56.h"
#include "Fonts/FreeSansBold12pt7b.h"
#include "MapleFreeRTOS1000.h"
#include "MapleFreeRTOS1000_pp.h"
#include "testSignal.h"
#include "dsoControl.h"
#include "HardwareSerial.h"
static void MainTask( void *a );
void splash(void);
//--
Adafruit_TFTLCD_8bit_STM32 *tft;
testSignal *myTestSignal;
DSOControl *controlButtons;
static uint16_t identifier=0;
static int counter=0;
//
// Test functions
//
extern void testTestSignal();
extern void testButtons();
/**
 * 
 */
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
    tft->setFontFamily(&FreeSansBold12pt7b, &FreeSansBold12pt7b, &FreeSansBold12pt7b);  
    tft->fillScreen(BLACK);
    
    splash();
    delay(500);
    myTestSignal=new testSignal(  PA7,PB12, 3,TIMER_CH2);
    myTestSignal->setFrequency(1000); // 1Khz
    
    controlButtons=new DSOControl ;
       
    
    // Ok let's go, switch to FreeRTOS
    xTaskCreate( MainTask, "MainTask", 500, NULL, 10, NULL );
    vTaskStartScheduler();      
}

void splash(void)
{
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
    controlButtons->setup();
    
   // testTestSignal();
     testButtons();   
}

//-
