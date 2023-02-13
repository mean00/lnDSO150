/**
    This is a bridge between the USB/CDC command parser and the main loop
    It is done indirectly through an event Queue to avoir race issue between the main thread and the USB/CDC thread
    As a bonus, it makes unplugging the USB/CDC part easier


*/
#include "lnArduino.h"
#include "dso_usbd_api.h"
#include "dso_capture.h"

    //evtGroup->setEvents(DSO_EVT_USB);
/**

*/
bool DSO_API::setVoltage(int a) 
{

    return true;
} //DSOCapture::DSO_VOLTAGE_RANGE r){}
/**

*/
bool DSO_API::setTimeBase(int a) 
{
    
    return true;
} //DSOCapture::DSO_TIME_BASE gb){}
/**

*/
bool DSO_API::setTrigger(int a) 
{
    return true;
} //DSOCapture::TriggerMode tm){}

int DSO_API::getVoltage()   { return (int)DSOCapture::getVoltageRange();    }
int DSO_API::getTimeBase()  { return (int)DSOCapture::getTimeBase;          }
int DSO_API::getTrigger()   { return (int)DSOCapture::getTriggerMode();     }

/**
DSOCapture::stopCapture();
DSOCapture::setTimeBase((DSOCapture::DSO_TIME_BASE )ctime);
DSOCapture::startCapture(240);

DSOCapture::stopCapture();
DSOCapture::setTriggerMode((DSOCapture::TriggerMode)v);
DSOCapture::startCapture(240);

DSOCapture::setVoltageRange((DSOCapture::DSO_VOLTAGE_RANGE )range);

*/
 
 void processUsbEvent()
{
}
