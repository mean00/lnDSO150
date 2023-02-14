/**
    This is a bridge between the USB/CDC command parser and the main loop
    It is done indirectly through an event Queue to avoir race issue between the main thread and the USB/CDC thread
    As a bonus, it makes unplugging the USB/CDC part easier


*/
#include "lnArduino.h"
#include "dso_usbd_api.h"
#include "dso_capture.h"
#include "messaging.pb.h"
#include "dso_events.h"

#define USB_QUEUE_SIZE 5

extern xFastEventGroup            *evtGroup;;

enum usb_queue_commands
{
     SET_VOLT       = 1,
     SET_TIMEBASE   = 2,
     SET_TRIGGER    = 3,
     SET_TRIGGER_VALUE = 4,
};

extern void redrawEverything();
extern void initUiEvent();

/**

*/
class UsbQueue
{
public:
        struct msg
        {
            uint32_t command;
            uint32_t value;
        };
        static UsbQueue *usb_queue;
        /**
        */
        UsbQueue()
        {
            _queue =  xQueueCreate( USB_QUEUE_SIZE, sizeof(msg)); // normally at most one in the queue...
                             
        }
        /**
        */
        bool pop_next(usb_queue_commands& cmd, uint32_t &val)
        {
            msg r;
             if (pdFALSE== xQueueReceive(
                               _queue,
                               &r,
                               0
                            ))
                {
                    return false;
                }
            cmd=(usb_queue_commands)r.command;
            val=r.value;
            return true;
        }
        /**
        */
        bool post(usb_queue_commands cmd, const uint32_t val)
        {
            msg r={cmd,val};            
            if(pdTRUE==xQueueSendToBack(
                               _queue,
                               &r,
                               0
                            ))
                {
                    evtGroup->setEvents(DSO_EVT_USB);
                    return true;
                }
            Logger("Cannot enqueue usb command\n");
            return false;
        }
        QueueHandle_t _queue;
};
//
UsbQueue *UsbQueue::usb_queue=NULL;
    //
/**
*/
void DSO_API::init( void)
{
    UsbQueue::usb_queue = new UsbQueue;
}
/**

*/
bool DSO_API::setVoltage(int a) 
{
    UsbQueue::usb_queue->post(SET_VOLT,a);
    return true;
}
/**

*/
bool DSO_API::setTimeBase(int a) 
{
    UsbQueue::usb_queue->post(SET_TIMEBASE,a);
    return true;
}
/**

*/
bool DSO_API::setTrigger(int a) 
{
    UsbQueue::usb_queue->post(SET_TRIGGER,a);
    return true;
} 

/**

*/
bool DSO_API::setTriggerValue(float a) 
{
    uint32_t  i=*(uint32_t *)&a;
    UsbQueue::usb_queue->post(SET_TRIGGER_VALUE,i);
    return true;
} 
int DSO_API::getVoltage()           { return (int)DSOCapture::getVoltageRange();    }
int DSO_API::getTimeBase()          { return (int)DSOCapture::getTimeBase();          }
int DSO_API::getTrigger()           { return (int)DSOCapture::getTriggerMode();     }
float DSO_API::getTriggerValue()    { return DSOCapture::getTriggerVoltage();     }

 
 void processUsbEvent()
{
    usb_queue_commands cmd;
    uint32_t val;
    while(UsbQueue::usb_queue->pop_next(cmd,val))
    {
        DSOCapture::stopCapture();
        switch(cmd)
        {
            case SET_VOLT:
                    DSOCapture::setVoltageRange((DSOCapture::DSO_VOLTAGE_RANGE )val);
                    break;
            case SET_TIMEBASE:
                    DSOCapture::setTimeBase((DSOCapture::DSO_TIME_BASE )val);
                    break;
            case SET_TRIGGER:            
                    DSOCapture::setTriggerMode((DSOCapture::TriggerMode)val);
                    break;
            case SET_TRIGGER_VALUE:
                    {
                    float f=*(float *)&val; // brute cast
                    DSOCapture::setTriggerVoltage(f);
                    break;
                    }
            default:
                xAssert(0);
                break;
        }
        redrawEverything();
        initUiEvent();
        DSOCapture::startCapture(240);
    }
}
// EOF
