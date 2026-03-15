/*
 *  (C) 2021 MEAN00 fixounet@free.fr
 *  See license file
 */
#pragma once
/**
 */
class lnCpuID
{
  public:
    enum LN_VENDOR
    {
        LN_VENDOR_UNKOWN,
        LN_VENDOR_STM,
        LN_VENDOR_GD,
        LN_VENDOR_WCH,
        LN_VENDOR_ESP
    };
    enum LN_MCU
    {
        LN_MCU_UNKNOWN,
        LN_MCU_ARM_F3,
        LN_MCU_ARM_F4,
        LN_MCU_RISCV,
        LN_MCU_XTENSA,
    };

  public:
    static void identify();
    static LN_VENDOR vendor()
    {
        return LN_VENDOR_STM;
    }
    static LN_MCU mcu()
    {
        return LN_MCU_ARM_F3;
    };
    static const char *mcuAsString()
    {
        return "dummy";
    };

    static int flashSize()
    {
        return 256 * 10024;
    };
    static int ramSize()
    {
        return 256 * 10024;
    };

    static const char *idAsString()
    {
        return "dummy";
    }
    static int clockSpeed()
    {
        return 1000 * 1000;
    };
};
