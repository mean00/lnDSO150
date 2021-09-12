#if 0
#include "Arduino.h"
#include "stdarg.h"
#include "embedded_printf/printf.h"
static char buffer[1024];
void Logger(const char *fmt...)
{
    va_list va;
    va_start(va,fmt);
    vsnprintf(buffer,1022,fmt,va);
    buffer[1023]=0;
    Serial1.print(buffer);
    Serial1.print("\n\r");
    va_end(va);
    
}
void Logger(int val)
{
    Serial1.print(val);
    
}
#endif