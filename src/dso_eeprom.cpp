/***************************************************
 STM32 duino based firmware for DSO SHELL/150
 *  * GPL v2
 * (c) mean 2019 fixounet@free.fr
 ****************************************************/
#include "EEPROM.h"
#include "dso_global.h"
#include "dso_eeprom.h"
#include "dso_adc_gain_priv.h"
extern uint16_t calibrationHash;

#define FINE_TUNE_OFFSET 64
#if 0
    #define CHECK_READ(x) xAssert(x)
#else
    #define CHECK_READ(x) if(!(x)) return false;
#endif
/**
 * 
 * @return 
 */
extern void *eeprom_begin;
static void addressInit(EEPROMClass &e2)
{
    uint32 pageBase0=(uint32_t)&eeprom_begin;    
    e2.init(pageBase0,pageBase0+2*1024,2*1024);
}
/**
 * 
 * @return 
 */
bool DSOEeprom::read()
{
    EEPROMClass e2;
    addressInit(e2);
    calibrationHash=e2.read(0);
    if(calibrationHash!=CURRENT_HASH)
    {
        return false;
    }
    
    for(int i=0;i<DSO_NB_GAIN_RANGES;i++)
            CHECK_READ(EEPROM_OK==e2.read(2+i,calibrationDC+i));
    for(int i=0;i<DSO_NB_GAIN_RANGES;i++)
            CHECK_READ(EEPROM_OK==e2.read(2+i+16,calibrationAC+i));
    // 15+2+16=33 so far
    return true;
}
/**
 * 
 * @return 
 */
bool  DSOEeprom::readFineVoltage()
{
    EEPROMClass e2;
    addressInit(e2);
    for(int i=0;i<DSO_NB_GAIN_RANGES;i++)
        voltageFineTune[i]=0;
    int hasFineVoltage=e2.read(FINE_TUNE_OFFSET);
    
    if(hasFineVoltage==CURRENT_HASH)
    {
        for(int i=0;i<DSO_NB_GAIN_RANGES;i++)
        {
            float f;
            uint16_t *adr=(uint16_t *)&f;
            uint16_t high,low;
            CHECK_READ(EEPROM_OK==e2.read(FINE_TUNE_OFFSET+2+i*2+0,&high)); // 64+4*2+  72 or 75
            CHECK_READ(EEPROM_OK==e2.read(FINE_TUNE_OFFSET+2+i*2+1,&low));
            
            adr[0]=high;
            adr[1]=low;
            voltageFineTune[i]=f;
        }
    }
    return true;
}
/**
 * 
 * @return 
 */
bool  DSOEeprom::write()
{
    EEPROMClass e2;
    addressInit(e2);
    
    for(int i=0;i<DSO_NB_GAIN_RANGES;i++)
        e2.write(2+i,calibrationDC[i]);
    for(int i=0;i<DSO_NB_GAIN_RANGES;i++)
        e2.write(2+i+16,calibrationAC[i]);
    
    e2.write(FINE_TUNE_OFFSET,CURRENT_HASH);
    for(int i=0;i<DSO_NB_GAIN_RANGES;i++)
    {
            float f=voltageFineTune[i];
            uint16_t *adr=(uint16_t *)&f;
            e2.write(FINE_TUNE_OFFSET+2+i*2+0,adr[0]);
            e2.write(FINE_TUNE_OFFSET+2+i*2+1,adr[1]);            
    }
    calibrationHash=e2.write(0,CURRENT_HASH);
    return true;
}
/**
 * 
 * @return 
 */
bool  DSOEeprom::wipe()
{
    EEPROMClass e2;
    addressInit(e2);
    e2.format();
    e2.write(0,0);
    e2.write(FINE_TUNE_OFFSET,0);
    return true;
}
bool  DSOEeprom::format()
{
    EEPROMClass e2;
    addressInit(e2);
    e2.format();
    return true;
}