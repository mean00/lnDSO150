
#include "lnArduino.h"
#include "dso_usbd_api.h"


bool DSO_API::setVoltage(int a)  //DSOCapture::DSO_VOLTAGE_RANGE r);
{
    printf("Got set voltage : %d\n",a );
    return true;
}
bool DSO_API::setTimeBase(int a)  //DSOCapture::DSO_TIME_BASE gb);
{
    printf("Got set time base : %d\n",a );
    return true;
}

bool DSO_API::setTrigger(int a)  //DSOCapture::TriggerMode tm);
{
    printf("Got set trigger : %d\n",a );
    return true;
}

int DSO_API::getVoltage()
{
    return 1;
}
int DSO_API::getTimeBase()
{
    return 1;
}
int DSO_API::getTrigger()
{
    return 1;
}

