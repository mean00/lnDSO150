#include "fancyLock.h"
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
   // xAssert(max<10000);
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
