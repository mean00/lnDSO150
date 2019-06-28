#include "MapleFreeRTOS1000_pp.h"
#pragma once


/**
 */
class FancyLock : public xMutex
{
public:
             FancyLock();
        void init();     
        bool lock();
        bool unlock();
protected:    
        int  max;
        int  start;
        int  maxWait;
};


class FancyInterrupts
{
public:
  static void disable();
  static void enable();
};

/**
 * 
 */
class FancySemaphore : public xBinarySemaphore
{
public:
        FancySemaphore();
        bool take();
        bool take(int timeoutMs);
protected:
        uint32_t start;
};