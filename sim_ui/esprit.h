
#pragma once
//
#include "stdint.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"
//
#include "printf.h"
//
#define DSO_SIM 1

#define Logger printf
#define xAssert(x)                                                                                                     \
    if (!(x))                                                                                                          \
    {                                                                                                                  \
        printf("Assert failed line %d file %s\n", __LINE__, __FILE__);                                                 \
        exit(-1);                                                                                                      \
    }

#define xDelay(n) usleep(n * 1000)
/*
 *
 *
 */
class lnFastEventGroup
{
  public:
    lnFastEventGroup()
    {
    }
    void takeOwnership()
    {
    }
    void setEvents(uint32_t b)
    {
    }
    uint32_t waitEvents(uint32_t b, uint32_t timeout)
    {
        return 0;
    }
};
/*
 *
 *
 */
class lnMutex
{
  public:
    lnMutex()
    {
    }
};
