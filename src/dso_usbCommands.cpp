#include "dso_usb.h"
#include "dso_debug.h"
#include "MapleFreeRTOS1000_pp.h"
#include "USBCompositeSerial.h"
#include "dso_usbCommands.h"
#include "dso_capture.h"
#include "DSO_config.h"
#include "dso_display.h"
extern DSOCapture                 *capture;
extern DSO_ArmingMode armingMode;
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
        void replyOk(int val)
        {
            _usbLock.lock();
             write32(((DSOUSB::ACK<<24)+(val&0xffff)));
             _usbLock.unlock();
        }
        virtual void    processCommand(uint32_t command);    
        void lock() {_usbLock.lock();}
        void unlock() {_usbLock.unlock();}
//protected:
        xQueueEvent _q;
        xMutex _usbLock;
};
extern USBCompositeSerial CompositeSerial;
UsbCommands *usbTask;
void uiSetVoltage(int v);
void uiSetTimeBase(int v);
void uiSetTriggerMode(int v);
void uiSetArmingMode(int v);
void uiRequestCapture(bool );
void uiSetTriggerValue(int v);
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
                case DSOUSB::VOLTAGE:     usbTask->replyOk(capture->getVoltageRange());return;
                case DSOUSB::TIMEBASE:    usbTask->replyOk(capture->getTimeBase());return;
                case DSOUSB::FIRMWARE:    usbTask->replyOk((DSO_VERSION_MAJOR<<8)+(DSO_VERSION_MINOR));return;
                case DSOUSB::TRIGGER:     usbTask->replyOk( (int) DSOCapture::getTriggerMode());return;                
                case DSOUSB::ARMINGMODE:  usbTask->replyOk(armingMode );return;       
                case DSOUSB::TRIGGERVALUE: usbTask->replyOk(capture->getTriggerValue()*100.+32768 );return;    
                case DSOUSB::DATA:                
                default:
                     usbTask->write32((DSOUSB::NACK<<24));
                     break;
            }
            return;
            break;
        case DSOUSB::SET:
            switch(target)
            {           

                case DSOUSB::VOLTAGE:     uiSetVoltage(value); usbTask->replyOk(0);return;
                case DSOUSB::TIMEBASE:    uiSetTimeBase(value);usbTask->replyOk(0);return;
                case DSOUSB::TRIGGER:     uiSetTriggerMode(value);usbTask->replyOk(0);return;
                case DSOUSB::ARMINGMODE:  uiSetArmingMode(value);usbTask->replyOk(0);return;               
                case DSOUSB::DATA:        uiRequestCapture(true);usbTask->replyOk(0);return;   
                case DSOUSB::TRIGGERVALUE:uiSetTriggerValue(value); usbTask->replyOk(0);return;
                default:
                    usbTask->write32((DSOUSB::NACK<<24));
                    break;
            }
            break;
        default:
            usbTask->write32((DSOUSB::NACK<<24));
            break;
            
    }
    return ;
}
void dsoUsb_sendData(int count,float *data, CaptureStats &stats)
{
    usbTask->lock();
    usbTask->write32(    (DSOUSB::EVENT<<24)+(DSOUSB::DATA<<16)+count);
    for(int i=0;i<count;i++)
    {
        usbTask->writeFloat(  data[i]);
    }
    
    usbTask->unlock();
    
}
// EOF
