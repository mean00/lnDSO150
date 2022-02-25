# We ony enable usb for arm and non small footprint
IF(NOT DEFINED LN_ARCH)
IF(1) # ARM
    IF(1)
        SET(LN_ARCH            "ARM" CACHE INTERNAL "")
        SET(LN_MCU             "M3"  CACHE INTERNAL "")
        SET(LN_MCU_RAM_SIZE    20    CACHE INTERNAL "")
        SET(LN_MCU_STATIC_RAM  4     CACHE INTERNAL "")
        IF(USE_SMALLFOOTPRINT)
          SET(LN_MCU_FLASH_SIZE  64   CACHE INTERNAL "")          
          SET(LN_MCU_EEPROM_SIZE 2     CACHE INTERNAL "")          
        ELSE()
          SET(LN_MCU_FLASH_SIZE  128    CACHE INTERNAL "")
          SET(LN_MCU_EEPROM_SIZE 4     CACHE INTERNAL "")                             
          SET(LN_ENABLE_USBD True CACHE INTERNAL "")

        ENDIF()
        SET(LN_SPEC         "picolibc"   CACHE INTERNAL "") # if not set we use nano
    ELSE(1)
        SET(LN_ARCH            "ARM" CACHE INTERNAL "")
        SET(LN_MCU             "M4"  CACHE INTERNAL "")
        SET(LN_MCU_FLASH_SIZE  256   CACHE INTERNAL "")
        SET(LN_MCU_RAM_SIZE    46    CACHE INTERNAL "")
        SET(LN_MCU_SPEED 108000000    CACHE INTERNAL "") #=> ok
        SET(LN_SPEC         "picolibc"   CACHE INTERNAL "") # if not set we use nano
        SET(LN_ENABLE_USBD True CACHE INTERNAL "")
        #SET(LN_MCU_SPEED 104000000  CACHE INTERNAL "")
        #SET(LN_MCU_SPEED 96000000  CACHE INTERNAL "")
    ENDIF()
ELSE() # RISCV
    SET(LN_ARCH "RISCV"              CACHE INTERNAL "")
    SET(LN_MCU  "VF103"              CACHE INTERNAL "")
    SET(LN_MCU_SPEED 108000000       CACHE INTERNAL "")
    SET(LN_MCU_RAM_SIZE    32        CACHE INTERNAL "")
    SET(LN_MCU_FLASH_SIZE  128       CACHE INTERNAL "")
    SET(LN_SPEC         "picolibc"   CACHE INTERNAL "") # if not set we use nano
    #SET(LN_MCU_SPEED 96000000  CACHE INTERNAL "")
    #SET(LN_MCU_SPEED 72000000  CACHE INTERNAL "")
ENDIF()
ENDIF(NOT DEFINED LN_ARCH)
MESSAGE(STATUS "Architecture ${LN_ARCH}, MCU=${LN_MCU}")
