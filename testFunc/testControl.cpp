#include "lnArduino.h"
#include "dso_control.h"

static xBinarySemaphore *sem;

 void TestControlCb(DSOControl::DSOEvent evt)
 {
     sem->give();
 }
void testControl()
{
    sem=new xBinarySemaphore;
    DSOControl *control2;
    control2=new DSOControl(TestControlCb);
    
    control2->setup();
    int c=0;
    while(1)
    {
        sem->take();
        c+=control2->getRotaryValue();
        Logger("Value=%d\n",c);
    }
}
