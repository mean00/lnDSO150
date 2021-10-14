#include "lnArduino.h"
#include "dso_control.h"

struct UI_eventCallbacks;
 typedef void redrawProto(bool onoff);
 typedef UI_eventCallbacks *nextProto();
 typedef void incdecProto(int count);
 extern DSOControl          *control;
 /**
  */
 
 struct UI_eventCallbacks
 {
     const redrawProto *redraw;
     const nextProto *next;
     const incdecProto *incdec;
 };
//-- 
void demo_redraw(bool onoff)
{

}
UI_eventCallbacks *demo_next()
{
    return NULL;
}
void  demo_incDec(int count)
{

}
//--
 
 const UI_eventCallbacks demoKey={demo_redraw,demo_next,demo_incDec};
 
 /**
  * 
  */
 void processUiEvent()
 {
     while(1)
     {
            int ev=control->getQButtonEvent();
            if(!ev) break;            
            int kind=ev>>16;
            int key=ev&0xffff;
            Logger("Got event %d on key %d\n",kind,key);
            
     }
     Logger("Rot=%d\n",control->getRotaryValue());
     
 }