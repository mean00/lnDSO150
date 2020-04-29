
#pragma once

typedef enum MCU_IDENTIFICATION
{
    MCU_STM32_64K,
    MCU_STM32_128K,
    MCU_GD32_64K,
    MCU_GD32_128K,
    MCU_GD32F3_256K
};

class cpuID
{
public:
        static void         identify();
        static uint32_t     getFamily();
        static uint32_t     getDesigner();
        static int          getFlashSize();
        static int          getRamSize();
        static MCU_IDENTIFICATION getId();
        static const char   *getIdAsString();
        
};
