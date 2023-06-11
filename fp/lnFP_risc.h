#pragma once
extern "C"
{
#include "rvfp/include/rvfplib.h"
}

#define LN_FP_MUL(a, b) __mulsf3(a, b)
#define LN_FP_ADD(a, b) __addsf3(a, b)
#define LN_TO_INT(a) __fixsfsi(a)
#define LN_FROM_INT(a) __floatsisf(a)
