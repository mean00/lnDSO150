/***************************************************
 STM32 duino based firmware for DSO SHELL/150
 *  * GPL v2
 * (c) mean 2019 fixounet@free.fr
 ****************************************************/
#include "EEPROM.h"
#include "dso_global.h"
#include "dso_eeprom.h"

extern uint16_t calibrationHash;
extern uint16_t calibrationDC[16];
extern uint16_t calibrationAC[16];
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
    return true;
}

