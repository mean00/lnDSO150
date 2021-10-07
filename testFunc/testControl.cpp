#include "lnArduino.h"
#include "dso_control.h"

 void TestControlCb(DSOControl::DSOEvent evt)
 {
     Logger("Event\n");
 }
void testControl()
{
    DSOControl *control2;
    control2=new DSOControl(TestControlCb);
    
    control2->setup();

    while(1)
    {
        xDelay(100);
    }
}
