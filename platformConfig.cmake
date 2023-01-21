IF(WIN32)    
    SET(PLATFORM_TOOLCHAIN_SUFFIX ".exe")
ENDIF(WIN32)

IF("${LN_ARCH}" STREQUAL "RISCV") # ---------  RISCV ----------
  SET(PLATFORM_PREFIX riscv32-unknown-elf-)
  SET(PLATFORM_C_FLAGS "-march=rv32imac -mabi=ilp32 -mcmodel=medlow")
  IF(WIN32)    
    SET(PLATFORM_TOOLCHAIN_PATH /c/gd32/toolchain/bin/) # Use /c/foo or c:\foo depending if you use mingw cmake or win32 cmake
  ELSE(WIN32)
    SET(PLATFORM_TOOLCHAIN_PATH /opt/gd32/toolchain/bin/)
  ENDIF(WIN32)
ELSE() # -------------  ARM -----------
   SET(PLATFORM_PREFIX arm-none-eabi-)
   SET(PLATFORM_C_FLAGS " ")
   IF(WIN32)
      SET(PLATFORM_TOOLCHAIN_PATH  "/c/dev/arm83/bin")
   ELSE()
     SET(PLATFORM_CLANG_PATH     "/arm/prebuilt1502/bin" CACHE INTERNAL "")
     SET(PLATFORM_TOOLCHAIN_PATH "/arm/113/bin" CACHE INTERNAL "")
     SET(PLATFORM_CLANG_VERSION} "-15" CACHE INTERNAL "")
   ENDIF()
ENDIF()

