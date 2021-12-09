#pragma once
extern "C"
{
#include "qfp/qfplib-m3.h"
}

#define LN_FP_MUL(a,b) qfp_fmul(a,b)
#define LN_FP_ADD(a,b) qfp_fadd(a,b)
#define LN_TO_INT(a)   (int)(a)
#define LN_FROM_INT(a) (float)(a)
