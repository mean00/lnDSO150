#include "lnArduino.h"
#include "dso_version.h"
#include "include/lnUsbStack.h"
#include "include/lnUsbCDC.h"
#include "cdc_descriptor.h"
#include "include/lnUsbDFUrt.h"


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
extern void lnSoftSystemReset(void);
extern void lnHardSystemReset(void);

void goDfu()
{
  Logger("Rebooting to DFU...\n");
  // pull DP to low
  lnDigitalWrite(PA12,0);
  lnPinMode(PA12,lnOUTPUT);
  lnDelayMs(100);
  volatile uint32_t *ram=(volatile uint32_t *)0x2000000;
  ram[2]=0x1234;
  ram[3]=0x5678;
  lnHardSystemReset(); 
  //lnSoftSystemReset();
}

/**
*/
void cdcEventHandler(void *cookie,int interface,lnUsbCDC::lnUsbCDCEvents event,uint32_t payload)
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
      case lnUsbCDC::CDC_SET_SPEED:
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
    lnUsbDFURT::addDFURTCb(goDfu);
    usb->start();
}

// EOF
