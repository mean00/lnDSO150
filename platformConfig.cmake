IF(WIN32)    
    SET(PLATFORM_TOOLCHAIN_SUFFIX ".exe")
ENDIF(WIN32)

IF("${LN_ARCH}" STREQUAL "RISCV") # RISCV
  SET(PLATFORM_PREFIX riscv32-unknown-elf-)
  SET(PLATFORM_C_FLAGS "-march=rv32imac -mabi=ilp32 -mcmodel=medlow")
  IF(WIN32)    
    SET(PLATFORM_TOOLCHAIN_PATH /c/gd32/toolchain/bin/) # Use /c/foo or c:\foo depending if you use mingw cmake or win32 cmake
  ELSE(WIN32)
    SET(PLATFORM_TOOLCHAIN_PATH /opt/gd32/toolchain2/bin/)
  ENDIF(WIN32)
ELSE()
   SET(PLATFORM_PREFIX arm-none-eabi-)
   SET(PLATFORM_C_FLAGS " ")
   IF(WIN32)
      SET(PLATFORM_TOOLCHAIN_PATH  "/c/dev/arm83/bin")
   ELSE()
      #SET(PLATFORM_TOOLCHAIN_PATH "/home/fx/Arduino_stm32/arm-gcc-2020q2/bin")
      SET(PLATFORM_TOOLCHAIN_PATH "/home/fx/Arduino_stm32/arm-gcc-2020q4/bin")
   ENDIF()
ENDIF()

