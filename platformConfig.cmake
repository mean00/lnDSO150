IF(WIN32)    
    SET(PLATFORM_TOOLCHAIN_SUFFIX ".exe")
ENDIF(WIN32)

IF("${LN_ARCH}" STREQUAL "RISCV") # RISCV
  IF( ${LN_MCU} STREQUAL "CH32V3x")    #----------CH32V307
    #SET(PLATFORM_PREFIX riscv-none-embed-) # MRS toolchain
    IF(WIN32)    
      SET(PLATFORM_TOOLCHAIN_PATH todo_todo) # Use /c/foo or c:\foo depending if you use mingw cmake or win32 cmake
    ELSE(WIN32)      
      #--- GCC ---------
      SET(PLATFORM_TOOLCHAIN_PATH "/riscv/tools5/bin" CACHE INTERNAL "")    
      IF(USE_HW_FPU)
        SET(PLATFORM_C_FLAGS "-march=rv32imafc -mabi=ilp32f " CACHE INTERNAL "")
      ELSE()
        SET(PLATFORM_C_FLAGS "-march=rv32imac -mabi=ilp32 " CACHE INTERNAL "")
      ENDIF()
      SET(PLATFORM_PREFIX "riscv32-unknown-elf-" CACHE INTERNAL "") 
      # FOR CLANG  
      
      SET(PLATFORM_TOOLCHAIN_TRIPLET "riscv32-unknown-elf-" CACHE INTERNAL "")      
      #-- CLANG --
      # No FPU
      #SET(PLATFORM_CLANG_PATH  "/riscv/tools_llvm/bin" CACHE INTERNAL "")
      SET(PLATFORM_CLANG_PATH  "/riscv/llvm_19.1/bin" CACHE INTERNAL "")
      SET(PLATFORM_CLANG_VERSION "-19")
      IF(USE_HW_FPU)
          SET(PLATFORM_CLANG_SYSROOT "${PLATFORM_CLANG_PATH}/../lib/clang-runtimes/riscv32-unknown-elf/riscv32_hard_fp/" CACHE INTERNAL "")
              #SET(PLATFORM_CLANG_SYSROOT "${PLATFORM_CLANG_PATH}/../lib/clang-runtimes/riscv32-none-eabi/riscv32_hard_fp/" CACHE INTERNAL "")
          SET(PLATFORM_CLANG_C_FLAGS "--target=riscv32 -march=rv32imafc -mabi=ilp32f  " CACHE INTERNAL "")      
      ELSE()
          SET(PLATFORM_CLANG_SYSROOT "${PLATFORM_CLANG_PATH}/../lib/clang-runtimes/riscv32-unknown-elf/riscv32_soft_nofp/" CACHE INTERNAL "")
              #SET(PLATFORM_CLANG_SYSROOT "${PLATFORM_CLANG_PATH}/../lib/clang-runtimes/riscv32-none-eabi/riscv32_soft_nofp/" CACHE INTERNAL "")
          SET(PLATFORM_CLANG_C_FLAGS "--target=riscv32 -march=rv32imac -mabi=ilp32  " CACHE INTERNAL "")      
      ENDIF()      
    ENDIF(WIN32)
  ELSE( ${LN_MCU} STREQUAL "CH32V3x") #----------- GD32VF103
    SET(PLATFORM_PREFIX riscv32-unknown-elf-)
    SET(PLATFORM_C_FLAGS "-march=rv32imac -mabi=ilp32 ")
    IF(WIN32)    
      SET(PLATFORM_TOOLCHAIN_PATH /c/gd32/toolchain/bin/) # Use /c/foo or c:\foo depending if you use mingw cmake or win32 cmake
    ELSE(WIN32)    
      SET(PLATFORM_TOOLCHAIN_PATH /opt/gd32/toolchain/bin/)
    ENDIF(WIN32)
  ENDIF()
ELSE()
   SET(PLATFORM_PREFIX arm-none-eabi-)
   SET(PLATFORM_C_FLAGS " ")
   IF(WIN32)
      SET(PLATFORM_TOOLCHAIN_PATH  "/c/dev/arm83/bin")
   ELSE()
           #SET(PLATFORM_TOOLCHAIN_PATH "/home/fx/Arduino_stm32/arm-gcc-2020q2/bin")
      #SET(PLATFORM_CLANG_PATH  /opt/llvm/bin)
      #SET(PLATFORM_CLANG_VERSION "-14")
      #SET(PLATFORM_CLANG_PATH  /opt/llvm_arm/LLVMEmbeddedToolchainForArm-14.0.0/bin)
      #SET(PLATFORM_CLANG_VERSION "")
      SET(PLATFORM_CLANG_PATH /arm/llvm_19.1/bin)
      SET(PLATFORM_CLANG_VERSION "-19")
      #SET(PLATFORM_CLANG_PATH  /opt/llvm_arm/14/bin)
      #SET(PLATFORM_CLANG_VERSION "")
      SET(PLATFORM_TOOLCHAIN_PATH "/arm/113/bin/")
      SET(PICO_SDK_PATH /pico/pico-sdk CACHE INTERNAL "") # For PICO2040

   ENDIF()
ENDIF()

