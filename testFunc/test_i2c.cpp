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
#include "HardwareSerial.h"
#include "AH_24Cxx.h"
#include "SoftWire.h"
#include "Wire.h"
#include "EEPROM.h"
// PA7 is timer3 channel2

extern Adafruit_TFTLCD_8bit_STM32 *tft;
extern testSignal *myTestSignal;




#define BUSADDRESS  0x50
// pb11 sda, pb10 scl

//SoftWire WIRE2 (PB10,PB11,SOFT_STANDARD);
TwoWire WIRE2(2,0);
static void scanner(WireBase &w);
/**
 * 
 * @param a
 */
uint8_t buff[16];
void testI2c( void)
{
    
    WIRE2.begin();
    
    scanner(WIRE2);
    AH_24Cxx ic_eeprom = AH_24Cxx(WIRE2,AT24C32, BUSADDRESS); // 24LC32A=>5   
    int adr=0;
    while(1)
    {
        
        ic_eeprom.read_buffer(adr,buff,16);   
        adr+=16;
        for(int i=0;i<16;i++) Serial.print(buff[i]);
        if(adr>4*1024) adr=0;
    };
    
}
void scanner(WireBase &w)
{
    int x=20;
    tft->setCursor(10,40);
    for(int adr=1;adr<127;adr++)
    {
        w.beginTransmission(adr);                                                                                                                                                                                                                                                                                                                                                                                                
        int error = w.endTransmission();   
        if(!error)
        {
            tft->println(adr);
            x++;
            tft->setCursor(10,10+x*20);
        }
    }
}
//-
