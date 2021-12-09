#if LN_ARCH == LN_ARCH_RISCV
#include "lnFP_risc.h"
#endif

#if LN_ARCH == LN_ARCH_ARM
    #ifdef LN_USE_FPU
        #define LN_FP_MUL(a,b) (a*b)
        #define LN_FP_ADD(a,b) (a+b)
        #define LN_TO_INT(a)   (int)(a)
        #define LN_FROM_INT(a) (float)(a)
    #else
        #include "lnFP_arm.h"
    #endif

#endif



