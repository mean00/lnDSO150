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
        bool giveFromInterrupt();
        bool give();
        bool reset();
protected:
        uint32_t start;
        int produced;
        int consumed;
        int nbReset;
        uint32_t lastTaken;
        uint32_t lastGiven;
};