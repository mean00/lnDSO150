
#pragma once
#include "lnArduino.h"

class lnUsbCDC
{
  public:
    enum lnUsbCDCEvents
    {
        CDC_DATA_AVAILABLE,
        CDC_SESSION_START,
        CDC_SESSION_END,
        CDC_SET_SPEED
    };

    typedef void lnUsbCDCEventsHandler(void *cookie, int interface, lnUsbCDCEvents event, uint32_t payload);

    int read(uint8_t *buffer, int maxSize);
    int write(const uint8_t *buffer, int maxSize);
    void flush();

  public:
    lnUsbCDC(int ev);
    void setEventHandler(lnUsbCDCEventsHandler *h, void *cookie);
    void clear_input_buffers();
};