/***************************************************
 STM32 duino based firmware for DSO SHELL/150
 *  * GPL v2
 * (c) mean 2019 fixounet@free.fr
 ****************************************************/

#include <Wire.h>
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_TFTLCD_8bit_STM32.h"
#include "Fonts/Targ56.h"
#include "Fonts/digitLcd56.h"
#include "Fonts/FreeSansBold12pt7b.h"
#include "MapleFreeRTOS1000.h"
#include "MapleFreeRTOS1000_pp.h"
#include "testSignal.h"
#include "dsoControl.h"
static void MainTask( void *a );
static void splash(void);


// PA7 is timer3 channel2

Adafruit_TFTLCD_8bit_STM32 *tft;
testSignal *myTestSignal;
DSOControl *controlButtons;
static uint16_t identifier=0;
static int counter=0;
/**
 * 
 */
void mySetup() 
{
    
    afio_cfg_debug_ports( AFIO_DEBUG_SW_ONLY); // Unlock PB3 & PB4
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
    tft->setFontFamily(&FreeSansBold12pt7b, &DIGIT_LCD56pt7b, &DIGIT_LCD56pt7b);
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

void splash()
{
        tft->setCursor(45, 10);
        tft->setTextColor(WHITE,BLACK);
        tft->setFontSize(Adafruit_TFTLCD_8bit_STM32::SmallFont);
        tft->myDrawString("    DSO-STM32duino");
        
}

void printButton(int b,const char *txt)
{
    if(controlButtons->getButtonState((DSOControl::DSOButton)b))
    {
        tft->setCursor(20, 30+b*25);     
        tft->println(txt); 
    }
     
}
void setTestSignal(int fq,bool high)
{
    static uint32_t bh, bl;
    
        tft->fillScreen(BLACK);   
        splash();
        myTestSignal->setFrequency(fq);
        myTestSignal->setAmplitute(high);
        tft->setCursor(20, 30);
        tft->println(fq);
        tft->setCursor(200, 30);
        tft->println(high);
        //
        //EXTI_BASE->IMR=3;
        for(int i=0;i<500;i++)
        {

            xDelay(10);
            printButton(3,"RotB    ");
            printButton(7,"Ok      ");
            printButton(6,"Trigger ");
            printButton(5,"Time    ");
            printButton(4,"Voltage ");
            int a=controlButtons->getRotaryValue();
            if(a)
            {
                counter+=a;
                tft->setCursor(200, 200);
                tft->println(counter);
            }
        }
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
    
    while(1)
    {
        setTestSignal(10000,true);
        setTestSignal(10000,false);

        setTestSignal(2000,true);
        setTestSignal(2000,false);
        setTestSignal(1000,true);
        setTestSignal(1000,false);
        setTestSignal(500,true);
        setTestSignal(500,false);

    }
        
}

//-
