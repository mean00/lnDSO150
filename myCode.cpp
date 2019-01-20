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
    if(!identifier) identifier=0x9341;
    tft=Adafruit_TFTLCD_8bit_STM32::spawn(0x7789);   
    if(!tft)
    {
        while(1) {};
    }
    tft->begin();
    tft->setRotation(1);
    tft->setFontFamily(&Targa56pt7b, &DIGIT_LCD56pt7b, &DIGIT_LCD56pt7b);
    tft->fillScreen(BLACK);
}
static uint32_t gpioA[10];
/**
 * 
 */
void myLoop(void) 
{  

    uint32_t start,end,org,mine;
#if 0    
    static int rotation=1;
    tft->setRotation(rotation);
    
    static int offset=0;
    offset=(offset+1)%15;
    
#if 0
    tft->setTextSize(0);
    tft->setFont(&DIGIT_LCD56pt7b);
    
#else
    tft->setFont(NULL);
    tft->setTextSize(10);
#endif    
    
    
    tft->setCursor(0, 0);
    tft->setTextColor(WHITE);  
    
    start=millis();
    tft->println("Hello");
    end=millis();
    org=end-start;
    
         
     
    tft->setTextColor(RED,BLUE);
    tft->setCursor(10+offset,140);
    tft->setFontSize(Adafruit_TFTLCD_8bit_STM32::SmallFont);
    start=millis();     
    tft->myDrawString("Hello",320-30);
    end=millis();
    
    tft->setTextSize(5);
    tft->setTextColor(WHITE,BLACK);
    tft->setFont(NULL);
    
    tft->setCursor(10, 70);
    tft->println(org);

    tft->setCursor(200, 70);
    
    tft->println(end-start);
    //delay(5000);
#endif
#if 0
    
#define ENC_BUTTON    0
#define VDIV_BUTTON   1
#define SECDIV_BUTTON 2
#define TRIG_BUTTON   3
#define OK_BUTTON     4


extern t_config config;
extern uint8_t currentFocus;
extern volatile bool hold;
extern volatile bool keepSampling;
extern volatile bool holdSampling;

volatile uint32_t pressedTime[5] = {0,0,0,0,0};
uint16_t pin[5] = {DB3_Pin,DB4_Pin,DB5_Pin,DB6_Pin,DB7_Pin};
GPIO_TypeDef* port[5] ={DB3_GPIO_Port,DB4_GPIO_Port,DB5_GPIO_Port,DB6_GPIO_Port,DB7_GPIO_Port};
    
#endif
    tft->setTextSize(3);
    start=millis();
    tft->fillScreen(BLACK);
    end=millis();
    tft->setCursor(200, 70);    
    tft->println(end-start);
    
    // Read GPIOB
    uint32_t val= GPIOB->regs->IDR;
    tft->setCursor(200, 90);    
#define CHECK_BUTTON(pin, txt) if(!(val & 1<<pin)) tft->println(txt);
    
    CHECK_BUTTON(3,"B ");
    CHECK_BUTTON(7,"Ok ");
    CHECK_BUTTON(6,"Tr ");
    CHECK_BUTTON(5,"Se ");
    CHECK_BUTTON(4,"Vt ");
    
    
    
    delay(200);
    
      
     
}


//-
