#include "lnArduino.h"
#include "dso_version.h"
#include "include/lnUsbStack.h"
#include "include/lnUsbCDC.h"
#include "cdc_descriptor.h"

lnUsbStack *usb =NULL;
lnUsbCDC *cdc=NULL;
#define MEVENT(x)        case lnUsbStack::USB_##x: Logger(#x); break;
/**
*/
void dsoUsbEvent(void *cookie, lnUsbStack::lnUsbStackEvents event)
{
    switch (event)
    {
        MEVENT(CONNECT)
        MEVENT(DISCONNECT)
        MEVENT(SUSPEND)
        MEVENT(RESUME)
        default: xAssert(0); break;
    }
}
/**
*/
void cdcEventHandler(void *cookie, int interface,lnUsbCDC::lnUsbCDCEvents event)
{
  uint8_t buffer[32];

    switch (event)
    {
      case lnUsbCDC::CDC_DATA_AVAILABLE:
        {
          int n=cdc->read(buffer,32);
          if(n)
          {
            cdc->write((uint8_t *)">",1);
            cdc->write(buffer,n);
          }
        }
          break;
      case lnUsbCDC::CDC_SESSION_START:
          Logger("CDC SESSION START\n");
          break;
      case lnUsbCDC::CDC_SESSION_END:
          Logger("CDC SESSION END\n");
          break;
      default:
          xAssert(0);
          break;
    }
}

/**
*/
void dsoInitUsb()
{
    usb = new lnUsbStack;
    usb->init(5, device_descriptor);
    usb->setConfiguration(desc_hs_configuration, desc_fs_configuration, &desc_device, &desc_device_qualifier);
    usb->setEventHandler(NULL, dsoUsbEvent);

    cdc=new lnUsbCDC(0);
    cdc->setEventHandler(cdcEventHandler,NULL);
    usb->start();
}

// EOF
