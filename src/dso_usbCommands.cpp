#include "dso_usb.h"
#include "dso_debug.h"
#include "MapleFreeRTOS1000_pp.h"
#include "USBCompositeSerial.h"
#include "dso_usbCommands.h"
#include "dso_capture.h"
#include "DSO_config.h"
extern DSOCapture                 *capture;

#define ZDEBUG Logger

/**
 * 
 * @param command
 */
class UsbCommands : public UsbTask
{
public:
        UsbCommands(const char *name,  int priority=2, int taskSize=100) :
                UsbTask(name,priority,taskSize) ,_q(10)
        {

        }
        virtual void    processCommand(uint32_t command);    
//protected:
        xQueueEvent _q;
};
extern USBCompositeSerial CompositeSerial;
UsbCommands *usbTask;
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
    ZDEBUG("Received command 0x%x type=%x target=%x value=%x\n",command,type,target,value);
    bool valid=true;
    if(type>=DSOUSB::COMMAND_LAST) valid=false;
    if(target>=DSOUSB::TARGET_LAST) valid=false;
    if(!valid)
    {
        ZDEBUG("Invalid command received\n");
        write32((DSOUSB::NACK<<24));
    }else
    {
        _q.post(command);
    }
    
}
/**
 * 
 * @param cmd
 * @return 
 */
void dsoUsb_processNextCommand()
{
    if(!usbTask) return  ;
    if(usbTask->_q.empty()) return  ;
    uint32_t cmd;
    usbTask->_q.get(0,cmd);
    
    int type=cmd>>24;
    int target=(cmd>>16)&0Xff;
    int value=(cmd&0xFFFF);
  
    switch(type)
    {
        case DSOUSB::GET:
            switch(target)
            {
                case DSOUSB::VOLTAGE:     usbTask->write32((DSOUSB::ACK<<24+capture->getVoltageRange()));return;
                case DSOUSB::TIMEBASE:    usbTask->write32((DSOUSB::ACK<<24+capture->getTimeBase()));return;
                case DSOUSB::FIRMWARE:    usbTask->write32((DSOUSB::ACK<<24+ (DSO_VERSION_MAJOR<<8)+(DSO_VERSION_MINOR)));return;
                case DSOUSB::TRIGGER:
                case DSOUSB::CAPTUREMODE:
                case DSOUSB::DATA:
                
                default:
                     usbTask->write32((DSOUSB::NACK<<24));
                     break;
            }
            return;
        default:
            usbTask->write32((DSOUSB::NACK<<24));
            break;
            
    }
    
    
    return ;
}

// EOF
