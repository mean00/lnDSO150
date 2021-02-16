#include "dso_usb.h"
#include "dso_debug.h"
#include "USBCompositeSerial.h"
#include "dso_usbCommands.h"
/**
 * 
 * @param command
 */
class UsbCommands : public UsbTask
{
public:
        UsbCommands(const char *name,  int priority=2, int taskSize=100): 
                UsbTask(name,priority,taskSize) 
        {

        }
        virtual void    processCommand(uint32_t command);    
};
extern USBCompositeSerial CompositeSerial;
UsbTask *usbTask;
/**
 * 
 */
void dso_usbInit()
{    
  //Reset the USB interface on generic boards - developed by Victor PV
  gpio_set_mode(PIN_MAP[PA12].gpio_device, PIN_MAP[PA12].gpio_bit, GPIO_OUTPUT_PP);
  gpio_write_bit(PIN_MAP[PA12].gpio_device, PIN_MAP[PA12].gpio_bit,0);

  for(volatile unsigned int i=0;i<512;i++);// Only small delay seems to be needed, and USB pins will get configured in Serial.begin
    gpio_set_mode(PIN_MAP[PA12].gpio_device, PIN_MAP[PA12].gpio_bit, GPIO_INPUT_FLOATING);

  USBComposite.setManufacturerString("MEAN00");
  USBComposite.setManufacturerString("DSO150DUINO");
  USBComposite.setSerialString("01234");        
  CompositeSerial.begin(115200);      
  usbTask=new UsbCommands("UsbControl",2,200);
}

/*
 
 Command
 *      1 Byte : Type  
 *              01: Get, 
 *              02: Set, 
 *              03: Event, 
 *              04: Result
 *      1 Byte : target
 *              01: Volt
 *              02: Time
 *              03: Trigger
 *              04: runmode (continuous / single)
 *      2 bytes value 
 */

/**
 * 
 * @param command
 */
void UsbCommands::processCommand(uint32_t command)
{
    int type=command>>24;
    int target=(command>>16)&0Xff;
    int value=(command&0xFFFF);
    Logger("Received command 0x%x type=%x target=%x value=%x\n",command,type,target,value);
    write32((DSOUSB::ACK<<24)+6);
    
}
