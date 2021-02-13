#include "stdlib.h"
#include "stdio.h"
#include "stdint.h"
#include "string.h"
#include "FreeRTOSConfig.h"
extern "C" void *pvPortMalloc(size_t sz);
extern "C" void vPortFree(void *pv);

uint8_t ucHeap[configTOTAL_HEAP_SIZE];

extern "C" void *calloc(size_t nb, size_t byte)
{
    int total=nb*byte;
    void *b=malloc(total);
    memset(b,0,total);
    return b;
}

extern "C" void *malloc(size_t byte)
{
    return pvPortMalloc(byte);
}
extern "C" void free(void *pv)
{
    if(!pv) return;
    vPortFree(pv);
}

