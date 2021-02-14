#include "USBComposite.h"
#include "USBCompositeSerial.h"
#include "MapleFreeRTOS1000_pp.h"
USBCompositeSerial CompositeSerial;
/**
 * 
 */
class UsbTask : public xTask
{
public:
                UsbTask(const char *name,  int priority=2, int taskSize=100): xTask(name,priority,taskSize)
                {
                    
                }
        void    run();
    
};

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
    
    usbTask=new UsbTask("UsbControl",2,200);
}
/**
 * 
 */
void UsbTask::run()
{
    while(1)
    {
        xDelay(10);
    }
}
// EOF
