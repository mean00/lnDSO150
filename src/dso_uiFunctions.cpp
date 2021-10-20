#include "lnArduino.h"
#include "dso_control.h"
#include "dso_display.h"

#include "dso_voltTime.h"
#include "dso_adc_capture.h"

struct UI_eventCallbacks;
 typedef void redrawProto(bool onoff);
 typedef void incdecProto(int count);
 extern DSOControl          *control;
 /**
  */


 struct UI_eventCallbacks
 {
     const int         myKey;
     const UI_eventCallbacks    *next;     
     const redrawProto  *redraw;     
     const incdecProto  *incdec;
 };
 
 
 int currentTrigger=0;
 
// Volt / Offset
 void voltMenu_redraw(bool on)
 {
     Logger("VolMenu : redraw %d\n",on);
     DSODisplay::drawVolt( DSOCapture::getVoltageRangeAsText(),on);
 }
 void voltMenu_incdec(int inc)
 {
     int  range=DSOCapture::getVoltageRange();
     range=(range+NB_CAPTURE_VOLTAGE+inc)%NB_CAPTURE_VOLTAGE;
     DSOCapture::setVoltageRange((DSOCapture::DSO_VOLTAGE_RANGE )range);
     Logger("VolMenu : %d\n",inc);
 }
// Volt / Offset
 void voltOffset_redraw(bool on)
 {
     Logger("voltOffset_redraw : redraw %d\n",on);
 }
 void voltOffset_incdec(int inc)
 {
     Logger("VolOffset : %d\n",inc);
 }
 //--
 // Volt / Offset
 void voltTrigger_redraw(bool on)
 {
     Logger("voltTrigger_redraw : redraw %d\n",on);
     DSODisplay::drawTrigger("Down",on);
 }
 void voltTrigger_incdec(int inc)
 {
     Logger("voltTrigger_incdec : %d\n",inc);
 }
 //-------
 // Volt / Offset
 void voltTriggerValue_redraw(bool on)
 {
     DSODisplay::drawVoltageTrigger(on, currentTrigger);
     Logger("voltTriggerValue_redraw : redraw %d\n",on);     
 }
 void voltTriggerValue_incdec(int inc)
 {
     Logger("voltTriggerValue_incdec : %d\n",inc);
     if(inc)
     {
        DSODisplay::drawVoltageTrigger(false, currentTrigger);
        currentTrigger+=inc;
        DSODisplay::drawVoltageTrigger(true, currentTrigger);
     }
     
     
 }
 //-------
 // Volt / Offset
 void time_redraw(bool on)
 {     
     Logger("voltTriggerValue_redraw : redraw %d\n",on);
     DSODisplay::drawTime(DSOCapture::getTimeBaseAsText(),on);
 }
 void time_incdec(int inc)
 {
    Logger("voltTriggerValue_incdec : %d\n",inc);
     
    int ctime=DSOCapture::getTimeBase();
    ctime=(ctime+inc+DSO_NB_TIMEBASE)%DSO_NB_TIMEBASE;
    DSOCapture::setTimeBase((DSOCapture::DSO_TIME_BASE )ctime);
 }
 
 
 
 
 //---------
 extern  const UI_eventCallbacks voltOffset,voltOkb,triggerValueMenu;
   const UI_eventCallbacks voltMenu=  {DSOControl::DSO_BUTTON_VOLTAGE,  &voltOffset, &voltMenu_redraw, &voltMenu_incdec};
   const UI_eventCallbacks voltOffset={DSOControl::DSO_BUTTON_VOLTAGE,  &voltMenu, &voltOffset_redraw, &voltOffset_incdec};

   const UI_eventCallbacks triggerMenu=      {DSOControl::DSO_BUTTON_TRIGGER, &triggerValueMenu,&voltTrigger_redraw,      &voltTrigger_incdec};
   const UI_eventCallbacks triggerValueMenu= {DSOControl::DSO_BUTTON_TRIGGER, &triggerMenu,     &voltTriggerValue_redraw, &voltTriggerValue_incdec};

   
      
   
   const UI_eventCallbacks timeMenu= {DSOControl::DSO_BUTTON_TIME,NULL, &time_redraw,  &time_incdec};

#define NB_TOP_MENU 8
   
 static const UI_eventCallbacks  *topMenus[NB_TOP_MENU]=
 {
     NULL,NULL,NULL, // up / down
     NULL,          //DSO_BUTTON_ROTARY= 3
     &voltMenu,     //DSO_BUTTON_VOLTAGE=4,
     &timeMenu,     //DSO_BUTTON_TIME=5,
     &triggerMenu,  //DSO_BUTTON_TRIGGER=6,
     NULL,          //DSO_BUTTON_OK=7
 };
 static const UI_eventCallbacks *currentMenu=NULL;
 
#if 1
    #define debug Logger
#else
    #define debug(...) {}
#endif
 /**
  * 
  */
 
 void initUiEvent()
 {
     for( int i=0;i<NB_TOP_MENU;i++)
     {
         const UI_eventCallbacks  *m=topMenus[i];
         if(m) 
             m->redraw(false);
     }
 }
 
 void processUiEvent()
 {
     while(1)
     {
            int ev=control->getQButtonEvent();
            if(!ev) break;            
            int kind=ev>>16;
            int key=ev&0xffff;
            debug("Event:%d , key:%d\n",kind,key);
           // if(kind==DSOControl::StatePressed)
            { // different key ?
                if(currentMenu)
                {
                    if(currentMenu->myKey==key)
                    {
                        debug("Toggle\n");
                        currentMenu->redraw(false); // toggle inside the same menu line
                        currentMenu=currentMenu->next;
                        if(currentMenu) currentMenu->redraw(true);
                        continue;
                    }
                }
                 
                if(currentMenu)
                      currentMenu->redraw(false);
                // it's a different menu line
                currentMenu=topMenus[key];
             //   xAssert(currentMenu);
                if(currentMenu)
                    currentMenu->redraw(true);                        
            }            
     }
     int incdec=control->getRotaryValue();
     debug("Rotary: %d\n",incdec);
     if(currentMenu)   
     {
         currentMenu->incdec(incdec);     
         currentMenu->redraw(true);
     }
 }