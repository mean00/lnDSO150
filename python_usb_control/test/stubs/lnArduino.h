#pragma once
#include "QThread"
#include "stdint.h"
#define xAssert(x)                                                                                                     \
    if (!(x))                                                                                                          \
    {                                                                                                                  \
        printf("%s FAILED\n", #x);                                                                                     \
        exit(-1);                                                                                                      \
    }

#define lnDigitalWrite(a, b)                                                                                           \
    {                                                                                                                  \
    }
#define lnPinMode(a, b)                                                                                                \
    {                                                                                                                  \
    }
#define PA12 0
#define PA13 0
#define lnOUTPUT 0

#define lnDelayMs(x) QThread::msleep(x);
#define Logger printf