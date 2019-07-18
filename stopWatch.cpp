

#include "dso_global.h"
#include "stopWatch.h"
#pragma once

/**
 */
bool StopWatch::elapsed(int threshold)
{
    if(!armed)
    {
        armed=true;
        mil=millis()&0xffff;
        return false;
    }
    uint32_t newMil=millis();
    if(mil<newMil)
    {
        return threshold<(newMil-mil);
    }
    return threshold<(0x10000+mil-newMil);
}

void StopWatch::ok()
{
    mil=millis();
}
