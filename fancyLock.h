#include "MapleFreeRTOS1000_pp.h"
#pragma once


/**
 */
class FancyLock : public xMutex
{
public:
             FancyLock();
        bool lock();
        bool unlock();
protected:    
        int  max;
        int  start;
};
/**
 */
class FancySemaphore : public xBinarySemaphore
{
public:
             FancySemaphore();
        bool take();
        bool take(int timeoutMs);
        bool give();
        bool giveFromInterrupt();
  
};

class FancyInterrupts
{
public:
  static void disable();
  static void enable();
};