

#include "dso_global.h"
#include "stopWatch.h"
#pragma once
static void tag();
/**
 * return true if the time since the last ok() is greater than threshold
 */
bool StopWatch::elapsed(int threshold)
{
    if(!armed)
    {
        armed=true;
        mil=millis()&0xffff;
        return false;
    }
    newMil=millis()&0xffff;
    int delta;
    if(mil<=newMil)
    {
        delta=newMil-mil;        
    }else
    {
        delta=(0x10000+mil)-newMil;
    }
    if (threshold<delta)
    {
      //  tag();
        return true;
    }
    return false;
}

void StopWatch::ok()
{
    mil=millis()&0xffff;
}

static void tag()
{
    xAssert(0);
}