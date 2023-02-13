

#include "dso_usbd_api.h"
bool DSO_API::setVoltage(int a) 
{
    return true;
} //DSOCapture::DSO_VOLTAGE_RANGE r){}
bool DSO_API::setTimeBase(int a) 
{
    return true;
} //DSOCapture::DSO_TIME_BASE gb){}
bool DSO_API::setTrigger(int a) 
{
    return true;
} //DSOCapture::TriggerMode tm){}

int DSO_API::getVoltage(){ return 1;}
int DSO_API::getTimeBase(){ return 2;}
int DSO_API::getTrigger(){ return 3;}

