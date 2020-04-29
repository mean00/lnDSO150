#include "Arduino.h"
#include "MapleFreeRTOS1000_pp.h"
#include "cpuID.h"
#include "scb.h"
static uint32_t family , designer;

static uint32_t _cpuId=0;
static int      _flashSize=0;
static int      _ramSize=0;
static MCU_IDENTIFICATION _chipId;

/**
 * 
 */
void cpuID::identify()
{
    struct scb_reg_map *scb=SCB_BASE;
    _cpuId=scb->CPUID;
   //   GD32: 41 2 f c23  1
    // Implementer 41 : ARM
    // VARIANT 2
    // Architecture C -> ARM V6
    // Partno :  0xC23 => Cortex M3
    // Rev=1
    
    
    // STM32F1 : 0x41 1 f c23 1
    // Variant 1
    // Architecture C -> ARM V6
    // Partno : C23  => Cortex M3
    // Rev=1
    
    // 2nd part ram & flash size
    uint32_t density=*(uint32_t *)0x1FFFF7E0;
    int ramSize=density>>16;
    _flashSize=density & 0xffff;
    if(ramSize!=0xffff) // GD
    {
        switch(_flashSize)
        {
            case 128:  _chipId=MCU_GD32_128K;break;
            case 64:   _chipId=MCU_GD32_64K;break;
            case 256 : _chipId=MCU_GD32F3_256K;break;
            default:
                        xAssert(0);
                        break;
        }
        _ramSize=ramSize;
    }else // STM32
    {
         if(_flashSize==128) 
            _chipId=MCU_STM32_128K;
        else if(_flashSize == 64)
                _chipId=MCU_STM32_64K;
            else
                xAssert(0);         
        _ramSize=20;
    }


}
/**
 * 
 * @return 
 */
uint32_t cpuID::getFamily()
{
    return family;
}
/**
 * 
 * @return 
 */
uint32_t cpuID::getDesigner()
{
    return designer;
}
/**
 * 
 * @return 
 */
int cpuID::getFlashSize()
{
    return _flashSize;
}
/**
 * 
 * @return 
 */
int cpuID::getRamSize()
{
    return _ramSize;
}
/**
 * 
 * @return 
 */
 MCU_IDENTIFICATION cpuID::getId()
 {
     return _chipId;
 }
 /**
  * 
  * @return 
  */
 const char *cpuID::getIdAsString()
 {
     switch(_chipId)
     {
            case MCU_STM32_64K: return "STM32F1/64kB"; break;
            case MCU_STM32_128K:return "STM32F1/128kB";break;
            case MCU_GD32_64K:return "GD32F1/64kB";break;
            case MCU_GD32_128K:return "GD32F1/128kB";break;
            case MCU_GD32F3_256K: return  "GD32F3/256kB";break;
            default:break;
     }
     xAssert(0);
 }