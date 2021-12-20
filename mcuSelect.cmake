IF(NOT DEFINED LN_ARCH)
IF(1) # ARM
    IF(1)
        SET(LN_ARCH            "ARM" CACHE INTERNAL "")
        SET(LN_MCU             "M3"  CACHE INTERNAL "")
        SET(LN_MCU_FLASH_SIZE  128   CACHE INTERNAL "")
        SET(LN_MCU_RAM_SIZE    20    CACHE INTERNAL "")
        SET(LN_SPEC         "picolibc"   CACHE INTERNAL "") # if not set we use nano
    ELSE()
        SET(LN_ARCH            "ARM" CACHE INTERNAL "")
        SET(LN_MCU             "M4"  CACHE INTERNAL "")
        SET(LN_MCU_FLASH_SIZE  256   CACHE INTERNAL "")
        SET(LN_MCU_RAM_SIZE    46    CACHE INTERNAL "")
        SET(LN_MCU_SPEED 72000000    CACHE INTERNAL "") #=> ok
        SET(LN_SPEC         "picolibc"   CACHE INTERNAL "") # if not set we use nano
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
