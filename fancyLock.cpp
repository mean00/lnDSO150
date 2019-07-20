#include "fancyLock.h"

#if 0
    #define CHECK_SLOW xAssert
#else
    #define CHECK_SLOW(...) {}
#endif
/**
 * 
 */
FancyLock::FancyLock()
{
    init();
}
/**
 * 
 */
void FancyLock::init()
{
    max=0;
    start=0;
    maxWait=0;
}
/**
 * 
 * @return 
 */
bool FancyLock::lock()
{
    int s=micros();
    bool r=xMutex::lock();    
    start=micros();
    int m=start-s;
    //CHECK_SLOW(m<20*1000);
    if(m>maxWait) maxWait=m;
    return r;
}
/**
 * 
 * @return 
 */
bool FancyLock::unlock()
{
    int m=micros()-start;
    if(m>max) max=m;
    CHECK_SLOW(max<10*1000);
    return xMutex::unlock();
    
    
}

/**
 */
int nesting=0;
int intCurrent=0;
int intMax=0;
/**
 * 
 */
void FancyInterrupts::disable()
{
    noInterrupts();
    if(!nesting)
        intCurrent=micros();
    nesting++;
    
    
}
/**
 * 
 */
void FancyInterrupts::enable()
{
    if(nesting==1)
    {
        int c=micros()-intCurrent;
        if(c>intMax) intMax=c;
      
    }
    nesting--;
    interrupts();
}

//--
FancySemaphore::FancySemaphore()
{
    start=0;
    produced=0;
    consumed=0;
    nbReset=0;
}
bool FancySemaphore::take()
{
    start=millis();
    bool r=xBinarySemaphore::take();
    CHECK_SLOW((millis()-start)<150);
    return r;
}
bool FancySemaphore::take(int timeoutMs)
{
    start=millis();
    bool r=xBinarySemaphore::take(timeoutMs);
    CHECK_SLOW((millis()-start)<150);
    if(r) 
    {
        consumed++;
        lastTaken=micros();
    }
    return r;
}
/**
 * 
 * @return 
 */
bool FancySemaphore::giveFromInterrupt()
{
    produced++;
    lastGiven=micros();
    return xBinarySemaphore::giveFromInterrupt();
}

/**
 * 
 * @return 
 */
bool FancySemaphore::give()
{
    produced++;
    lastGiven=micros();
    return xBinarySemaphore::give();
}

bool FancySemaphore::reset()
{
    if(take(1))
    {
     
        give();
    }
    nbReset++;
    return true;
}

// EOF
