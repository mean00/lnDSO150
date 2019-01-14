/*
    Thin wrapper on top of FreeRTOS to be more C++ friendly
 */

#include "MapleFreeRTOS1000_pp.h"

void do_assert(const char *a)
{
    
    Serial.println(a);
    while(1)
    {
        
    };
    
}
#define xAssert(a) if(!(a)) {do_assert(#a);}


//-- Binary Semaphor --

/**
 * 
 */
xBinarySemaphore::xBinarySemaphore()
{
    _handle=xSemaphoreCreateBinary();
    xAssert(_handle);
}
/**
 * 
 * @return 
 */
bool xBinarySemaphore::take()
{
  return (bool) xSemaphoreTake(_handle,portMAX_DELAY);
}
/**
 * 
 * @param timeoutMs
 * @return 
 */
bool xBinarySemaphore::take(int timeoutMs)
{
  int ticks=1+(timeoutMs*configTICK_RATE_HZ+500)/1000;
  return (bool) xSemaphoreTake(_handle,ticks);
}
/**
 * 
 * @return 
 */
 bool xBinarySemaphore::give()
{
  xAssert(xSemaphoreGive(_handle)); // should never fail
  return true;
}
/**
 * 
 * @return 
 */
 bool xBinarySemaphore::giveFromInterrupt()
{
  BaseType_t awake;
  return (xSemaphoreGiveFromISR(_handle,&awake)); // should never fail... 
}
 /**
  * 
  * @param ms
  */
 void xDelay(int ms)
 {
      const TickType_t xDelay = 1+( ms/ portTICK_PERIOD_MS);
      vTaskDelay(xDelay);
 }

 
 xMutex::xMutex()
 {
     _handle=xSemaphoreCreateMutex();
 }
 bool xMutex::lock()
 {
    xAssert(xSemaphoreTake(_handle,portMAX_DELAY)); // should never fail
    return true;
 }
 bool xMutex::unlock()
 {
    xAssert(xSemaphoreGive(_handle)); // should never fail
    return true;
 }
 
 //EOF