IF(WIN32)
  SET(PLATFORM_TOOLCHAIN_SUFFIX ".exe")
ENDIF()

IF("${LN_ARCH}" STREQUAL "RISCV") # RISCV
  IF( ${LN_MCU} STREQUAL "CH32V3x")    #----------CH32V307
    #SET(PLATFORM_PREFIX riscv-none-embed-) # MRS toolchain
    IF(WIN32)
      SET(PLATFORM_TOOLCHAIN_PATH todo_todo) # Use /c/foo or c:\foo depending if you use mingw cmake or win32 cmake
    ELSE()
      #--- GCC ---------
      SET(GCC_OPTIM "-msave-restore")
      SET(PLATFORM_TOOLCHAIN_PATH "/riscv/xpack-14.2.0-2/bin" CACHE INTERNAL "")
      IF(USE_HW_FPU)
        SET(PLATFORM_C_FLAGS "-march=rv32imafc_zicsr -mabi=ilp32f ${GCC_OPTIM}" CACHE INTERNAL "")
      ELSE()
        SET(PLATFORM_C_FLAGS "-march=rv32imac_zicsr -mabi=ilp32  ${GCC_OPTIM}" CACHE INTERNAL "")
      ENDIF()
      SET(PLATFORM_PREFIX "riscv-none-elf-" CACHE INTERNAL "")
      # FOR CLANG

      SET(PLATFORM_TOOLCHAIN_TRIPLET "riscv-none-elf-" CACHE INTERNAL "")
      #-- CLANG --
      # No FPU
      #SET(PLATFORM_CLANG_PATH  "/riscv/tools_llvm/bin" CACHE INTERNAL "")
      if(TRUE)
        SET(PLATFORM_CLANG_PATH  "/riscv/llvm_21.1.8/bin" CACHE INTERNAL "")
        SET(PLATFORM_CLANG_VERSION "-21")
      else()
        SET(PLATFORM_CLANG_PATH  "/riscv/21/bin" CACHE INTERNAL "")
        SET(PLATFORM_CLANG_VERSION "-21")
      endif()
      IF(USE_HW_FPU)
        #SET(PLATFORM_CLANG_SYSROOT "${PLATFORM_CLANG_PATH}/../lib/clang-runtimes/riscv32-none-eabi/riscv32_hard_fp/" CACHE INTERNAL "")
        SET(PLATFORM_CLANG_SYSROOT "${PLATFORM_CLANG_PATH}/../lib/clang-runtimes/riscv32-unknown-elf/riscv32_hard_fp/" CACHE INTERNAL "")
        SET(PLATFORM_CLANG_C_FLAGS "--target=riscv32 -march=rv32imafc_zicsr -mabi=ilp32f -I${PLATFORM_CLANG_PATH}/../include " CACHE INTERNAL "")
      ELSE()
        #SET(PLATFORM_CLANG_SYSROOT "${PLATFORM_CLANG_PATH}/../lib/clang-runtimes/riscv32-none-eabi/riscv32_soft_nofp/" CACHE INTERNAL "")
        SET(PLATFORM_CLANG_SYSROOT "${PLATFORM_CLANG_PATH}/../lib/clang-runtimes/riscv32-unknown-elf/riscv32_soft_nofp/" CACHE INTERNAL "")
        SET(PLATFORM_CLANG_C_FLAGS "--target=riscv32 -march=rv32imac_zicsr -mabi=ilp32  -I${PLATFORM_CLANG_PATH}/../include" CACHE INTERNAL "")

      ENDIF()
    ENDIF()
  ELSE() #----------- GD32VF103
    SET(PLATFORM_PREFIX riscv32-unknown-elf-)
    SET(PLATFORM_C_FLAGS "-march=rv32imac -mabi=ilp32 ")
    IF(WIN32)
      SET(PLATFORM_TOOLCHAIN_PATH /c/gd32/toolchain/bin/) # Use /c/foo or c:\foo depending if you use mingw cmake or win32 cmake
    ELSE()
      SET(PLATFORM_TOOLCHAIN_PATH /opt/gd32/toolchain/bin/)
    ENDIF()
  ENDIF()
ELSE()
  SET(PLATFORM_PREFIX arm-none-eabi-)
  SET(PLATFORM_C_FLAGS " ")
  IF(WIN32)
    SET(PLATFORM_TOOLCHAIN_PATH  "/c/dev/arm83/bin")
  ELSE()
    #/arm/arm_llvm_21.1.1/lib/clang-runtimes/arm-none-eabi
    #SET(PLATFORM_CLANG_PATH /arm/arm_llvm_21.1.1/bin)
    #SET(PLATFORM_CLANG_VERSION "-21")
    SET(PLATFORM_CLANG_PATH /arm/llvm_21.1/bin)
    SET(PLATFORM_CLANG_VERSION "-21")
    #SET(PLATFORM_CLANG_PATH  /opt/llvm_arm/14/bin)
    #SET(PLATFORM_CLANG_VERSION "")
    SET(PLATFORM_TOOLCHAIN_PATH "/arm/14.2/bin/")
    SET(PICO_SDK_PATH /opt/pico/pico-sdk CACHE INTERNAL "") # For PICO2040

  ENDIF()
ENDIF()

