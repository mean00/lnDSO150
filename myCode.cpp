/***************************************************
 Digitally controlled power supply
 *  * GPL v2
 * (c) mean 2018 fixounet@free.fr
 ****************************************************/

#include <Wire.h>
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_TFTLCD_8bit_STM32.h"
#include "Fonts/Targ56.h"
#include "Fonts/digitLcd56.h"
#include "Fonts/FreeSansBold12pt7b.h"

// overlaping:
#define XM TFT_RS // 330 Ohm // must be an analog pin !!!
#define YP TFT_CS // 500 Ohm // must be an analog pin !!!
#define XP PB0 //TFT_D0 // 330 Ohm // can be a digital pin
#define YM PB1 //TFT_D1 // 500 Ohm // can be a digital pin


Adafruit_TFTLCD_8bit_STM32 *tft;
static uint16_t identifier=0;
/**
 * 
 */
void mySetup() 
{
    
    afio_cfg_debug_ports( AFIO_DEBUG_SW_ONLY); // Unlock PB3 & PB4
    Serial.println("Init"); 
    
    identifier = tft->readID();
    tft=Adafruit_TFTLCD_8bit_STM32::spawn(identifier);    
    tft->begin();
    tft->setRotation(1);
    tft->setFontFamily(&Targa56pt7b, &DIGIT_LCD56pt7b, &DIGIT_LCD56pt7b);
}
static uint32_t gpioA[10];
/**
 * 
 */
void myLoop(void) 
{    
    uint32_t start,end,org,mine;
    static int rotation=1;
    tft->setRotation(rotation);
    
#if 0
    tft->setTextSize(0);
    tft->setFont(&DIGIT_LCD56pt7b);
    
#else
    tft->setFont(NULL);
    tft->setTextSize(10);
#endif    
    
    tft->fillScreen(BLACK);
    tft->setCursor(0, 0);
    tft->setTextColor(WHITE);  
    
    start=millis();
    tft->println("Hello");
    end=millis();
    org=end-start;
    
         
     
    tft->setTextColor(RED,BLUE);
    tft->setCursor(10,140);
    tft->setFontSize(Adafruit_TFTLCD_8bit_STM32::SmallFont);
    start=millis();     
    tft->myDrawString("Hello");
    end=millis();
    
    tft->setTextSize(5);
    tft->setTextColor(WHITE,BLACK);
    tft->setFont(NULL);
    
    tft->setCursor(10, 70);
    tft->println(org);

    tft->setCursor(200, 70);
    
    tft->println(end-start);
    delay(5000);
}


//-
