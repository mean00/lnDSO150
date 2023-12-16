# We ony enable usb for arm and non small footprint
IF(NOT DEFINED LN_ARCH)
  IF(USE_CH32V3x) # RISCV
    # try without crystal...
    #SET(LN_USE_INTERNAL_CLOCK True        CACHE INTERNAL "")
    #SET(LN_MCU_SPEED          48000000    CACHE INTERNAL "") # 96 Mhz
    #SET(LN_MCU_SPEED          96000000    CACHE INTERNAL "") # 96 Mhz
    SET(LN_MCU_SPEED          144000000    CACHE INTERNAL "") # 144 Mhz

    SET(LN_ARCH "RISCV"                   CACHE INTERNAL "")
    SET(LN_MCU  "CH32V3x"                 CACHE INTERNAL "")
    SET(LN_MCU_RAM_SIZE       64          CACHE INTERNAL "")
    SET(LN_MCU_FLASH_SIZE     256         CACHE INTERNAL "")
    SET(LN_MCU_STATIC_RAM     10           CACHE INTERNAL "")
    SET(LN_SPEC               "picolibc"  CACHE INTERNAL "") # if not set we use nano
    SET(LN_BOOTLOADER_SIZE     16          CACHE INTERNAL "")
    #SET(LN_SPEC         "picolibc"   CACHE INTERNAL "") # if not set we use nano
  ELSE()
    IF(USE_GD32F3)
        SET(LN_ARCH            "ARM" CACHE INTERNAL "")
        SET(LN_MCU             "M4"  CACHE INTERNAL "")
        SET(LN_MCU_FLASH_SIZE  256   CACHE INTERNAL "")
        SET(LN_MCU_RAM_SIZE    46    CACHE INTERNAL "")
        SET(LN_MCU_STATIC_RAM  7     CACHE INTERNAL "")
        #SET(LN_MCU_SPEED 72000000    CACHE INTERNAL "") #=> ok
        SET(LN_MCU_SPEED 96000000    CACHE INTERNAL "") #=> ok
        
        #      SET(LN_MCU_SPEED  72000000    CACHE INTERNAL "") #=> ok
        SET(LN_SPEC         "picolibc"   CACHE INTERNAL "") # if not set we use nano
        SET(LN_ENABLE_USBD True CACHE INTERNAL "")
        SET(LN_BOOTLOADER_SIZE 8     CACHE INTERNAL "")
    ELSEIF(USE_RP2040)
        SET(LN_ARCH            "ARM" CACHE INTERNAL "")
        SET(LN_MCU             "RP2040"  CACHE INTERNAL "")
        SET(LN_MCU_RAM_SIZE    200    CACHE INTERNAL "")
        SET(LN_MCU_SPEED       72000000       CACHE INTERNAL "")
        SET(LN_MCU_STATIC_RAM  10     CACHE INTERNAL "")
        SET(LN_MCU_FLASH_SIZE  2048   CACHE INTERNAL "")
        SET(LN_MCU_EEPROM_SIZE 2     CACHE INTERNAL "")
        SET(LN_BOOTLOADER_SIZE 8     CACHE INTERNAL "")        
        SET(LN_SPEC            "picolibc"   CACHE INTERNAL "") # if not set we use nano
        
    ELSE()
        SET(LN_ARCH            "ARM" CACHE INTERNAL "")
        SET(LN_MCU             "M3"  CACHE INTERNAL "")
        SET(LN_MCU_RAM_SIZE    20    CACHE INTERNAL "")
        SET(LN_MCU_SPEED 72000000       CACHE INTERNAL "")
        IF(USE_SMALLFOOTPRINT)
          SET(LN_MCU_STATIC_RAM  3    CACHE INTERNAL "")
          SET(LN_MCU_FLASH_SIZE  64   CACHE INTERNAL "")
          SET(LN_MCU_EEPROM_SIZE 2    CACHE INTERNAL "")
        ELSE() # "Big" flash
          SET(LN_MCU_STATIC_RAM  5     CACHE INTERNAL "")
          SET(LN_MCU_FLASH_SIZE  128   CACHE INTERNAL "")
          SET(LN_MCU_EEPROM_SIZE 2     CACHE INTERNAL "")
          SET(LN_BOOTLOADER_SIZE 8     CACHE INTERNAL "")
          SET(LN_ENABLE_USBD True CACHE INTERNAL "")
        ENDIF()
        SET(LN_SPEC         "picolibc"   CACHE INTERNAL "") # if not set we use nano
    ENDIF() # GD32F3
  ENDIF() # CH32V3
ENDIF(NOT DEFINED LN_ARCH)
MESSAGE(STATUS "Architecture ${LN_ARCH}, MCU=${LN_MCU}")
