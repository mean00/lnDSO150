

IF("${LN_ARCH}" STREQUAL "RISCV")
    SET(FP_SRC 
            rvfp/src/asm/ieee754_nd/__adddf3.S       rvfp/src/asm/ieee754_nd/__fixunsdfsi.S   rvfp/src/asm/ieee754_nd/__gesf2.S
            rvfp/src/asm/ieee754_nd/__addsf3.S       rvfp/src/asm/ieee754_nd/__fixunssfdi.S   rvfp/src/asm/ieee754_nd/__gtdf2.S
            rvfp/src/asm/ieee754_nd/__divdf3.S       rvfp/src/asm/ieee754_nd/__fixunssfsi.S   rvfp/src/asm/ieee754_nd/__gtsf2.S
            rvfp/src/asm/ieee754_nd/__divsf3.S       rvfp/src/asm/ieee754_nd/__floatdidf.S    rvfp/src/asm/ieee754_nd/__ledf2.S
            rvfp/src/asm/ieee754_nd/__eqdf2.S        rvfp/src/asm/ieee754_nd/__floatdisf.S    rvfp/src/asm/ieee754_nd/__lesf2.S
            rvfp/src/asm/ieee754_nd/__eqsf2.S        rvfp/src/asm/ieee754_nd/__floatsidf.S   # rvfp/src/asm/ieee754_nd/__ltdf2.S
            rvfp/src/asm/ieee754_nd/__extendsfdf2.S  rvfp/src/asm/ieee754_nd/__floatsisf.S    rvfp/src/asm/ieee754_nd/__ltsf2.S
            rvfp/src/asm/ieee754_nd/__fixdfdi.S      rvfp/src/asm/ieee754_nd/__floatundidf.S  rvfp/src/asm/ieee754_nd/__muldf3.S
            rvfp/src/asm/ieee754_nd/__fixdfsi.S      rvfp/src/asm/ieee754_nd/__floatundisf.S  rvfp/src/asm/ieee754_nd/__mulsf3.S
            rvfp/src/asm/ieee754_nd/__fixsfdi.S      rvfp/src/asm/ieee754_nd/__floatunsidf.S  rvfp/src/asm/ieee754_nd/__subdf3.S
            rvfp/src/asm/ieee754_nd/__fixsfsi.S      rvfp/src/asm/ieee754_nd/__floatunsisf.S  rvfp/src/asm/ieee754_nd/__subsf3.S
            rvfp/src/asm/ieee754_nd/__fixunsdfdi.S   rvfp/src/asm/ieee754_nd/__gedf2.S        rvfp/src/asm/ieee754_nd/__truncdfsf2.S)
    ADD_DEFINITIONS("-DPERFORMANCE=1")
    add_library(rvfp ${FP_SRC})
ENDIF("${LN_ARCH}" STREQUAL "RISCV")

IF("${LN_ARCH}" STREQUAL "ARM")
      IF("${LN_MCU}" STREQUAL "M3")
      #  IF(USE_CLANG)
      #    MESSAGE(STATUS "Clang is being used, disabling asm floating point")
      #  ELSE(USE_CLANG)
          add_subdirectory(qfp)
      #  ENDIF(USE_CLANG)
      ENDIF("${LN_MCU}" STREQUAL "M3")
ENDIF("${LN_ARCH}" STREQUAL "ARM")


