/***************************************************
 STM32 duino based firmware for DSO SHELL/150
 *  * GPL v2
 * (c) mean 2019 fixounet@free.fr
 ****************************************************/
#include "EEPROM.h"
#include "dso_global.h"
#include "dso_eeprom.h"

extern uint16_t calibrationHash;
extern uint16_t calibrationDC[NB_DSO_VOLTAGE+1];
extern uint16_t calibrationAC[NB_DSO_VOLTAGE+1];
extern float    voltageFineTune[NB_DSO_VOLTAGE+1];

#define FINE_TUNE_OFFSET 64

/**
 * 
 * @return 
 */
bool  DSOEeprom::read()
{
    EEPROMClass e2;
    e2.init();
    calibrationHash=e2.read(0);
    if(calibrationHash!=CURRENT_HASH)
    {
        return false;
    }
    
    for(int i=0;i<16;i++)
        calibrationDC[i]=e2.read(2+i);
    for(int i=0;i<16;i++)
        calibrationAC[i]=e2.read(2+i+16);
    // 15+2+16=33 so far
    for(int i=0;i<16;i++)
        voltageFineTune[i]=0;
    int hasFineVoltage=e2.read(FINE_TUNE_OFFSET);
    
    if(hasFineVoltage==CURRENT_HASH)
    {
        for(int i=0;i<16;i++)
        {
            float f;
            uint16_t *adr=(uint16_t *)&f;
            uint16_t high=e2.read(FINE_TUNE_OFFSET+2+i*2+0);
            uint16_t low=e2.read(FINE_TUNE_OFFSET+2+i*2+1);
            
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
    e2.init();
    e2.format();
    e2.write(0,CURRENT_HASH);
    calibrationHash=e2.read(0);
    for(int i=0;i<16;i++)
        e2.write(2+i,calibrationDC[i]);
    for(int i=0;i<16;i++)
        e2.write(2+i+16,calibrationAC[i]);
    
    e2.write(FINE_TUNE_OFFSET,CURRENT_HASH);
    for(int i=0;i<16;i++)
    {
            float f=voltageFineTune[i];
            uint16_t *adr=(uint16_t *)&f;
            e2.write(FINE_TUNE_OFFSET+2+i*2+0,adr[0]);
            e2.write(FINE_TUNE_OFFSET+2+i*2+1,adr[1]);            
    }
    return true;
}
/**
 * 
 * @return 
 */
bool  DSOEeprom::wipe()
{
    EEPROMClass e2;
    e2.init();
    e2.format();
    e2.write(0,0);
    e2.write(64,0);
    return true;
}

