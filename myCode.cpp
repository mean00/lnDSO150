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
#include "MapleFreeRTOS1000.h"
#include "MapleFreeRTOS1000_pp.h"
#include "Rotary.h"
static void MainTask( void *a );
static void splash(void);
HardwareTimer pwmtimer(3);

// PA7 is timer3 channel2

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
    
    
    // square freq tester is A7    
    
    pinMode(PA7,PWM);  
    pwmtimer.pause();
    pwmtimer.setPrescaleFactor(18);
    pwmtimer.setCount(0);
    pwmtimer.setOverflow(4000);
    pwmtimer.setCompare(TIMER_CH2, 2000);  
    pwmtimer.refresh();
    pwmtimer.resume();

#if 0
    if(0)
    {
        pinMode(PA12,OUTPUT); // AmpSel, 0.3v
        digitalWrite(PA12,1);
    }
    else
    {
        pinMode(PA12,INPUT_PULLUP); // AmpSel
    }
#endif
    
    
    
   // Ok let's go, switch to FreeRTOS
   xTaskCreate( MainTask, "MainTask", 500, NULL, 10, NULL );
   vTaskStartScheduler();      
}

void splash()
{
        tft->setCursor(10, 10);
        tft->setTextColor(WHITE,BLACK);
        tft->setFontSize(Adafruit_TFTLCD_8bit_STM32::SmallFont);
        tft->myDrawString("DSO-STM32duino");
        delay(500);
        tft->fillScreen(BLACK);
}

/**
 * 
 * @param a
 */
void MainTask( void *a )
{
    tft->setTextSize(5);
    while(1)
    {
        xDelay(10);
        static int refresh=0;

        refresh++;
        if(refresh>=300)
        {
            // Redraw
             tft->fillScreen(BLACK);
             refresh=0;
        }
         // Read GPIOB
        uint32_t val= GPIOB->regs->IDR;
        tft->setCursor(200, 90);    
    #define CHECK_BUTTON(pin, txt) if(!(val & 1<<pin))  {tft->setCursor(200, 30*(pin-3));     tft->println(txt);}

        CHECK_BUTTON(3,"B ");
        CHECK_BUTTON(7,"Ok ");
        CHECK_BUTTON(6,"Tr ");
        CHECK_BUTTON(5,"Se ");
        CHECK_BUTTON(4,"Vt ");
    }
}


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
