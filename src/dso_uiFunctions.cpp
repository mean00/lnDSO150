#include "lnArduino.h"
#include "dso_control.h"
#include "dso_display.h"

#include "dso_capture.h"

struct UI_eventCallbacks;
 typedef void redrawProto(bool onoff);
 typedef void incdecProto(int count);
 extern DSOControl          *control;
 extern float voltageOffset;
 extern void redrawEverything();
 extern void autoSetup();
 /**
  */


 struct UI_eventCallbacks
 {
     const int         myKey;
     const UI_eventCallbacks    *next;     
     const redrawProto  *redraw;     
     const incdecProto  *incdec;
 };
 
 
 extern void menuManagement(DSOControl *control) ;
// Volt / Offset
 void voltMenu_redraw(bool on)
 {
     Logger("VolMenu : redraw %d\n",on);
     DSODisplay::drawVolt( DSOCapture::getVoltageRangeAsText(),on);
 }
 /**
  */
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
      DSODisplay::printOffsetValue( voltageOffset,on);
 }
 /*
  */
 void voltOffset_incdec(int inc)
 {
     float finc=((float)inc)*0.1;
     voltageOffset+=finc;     
 }
 //--
 // Volt / Offset
 void voltTrigger_redraw(bool on)
 {
     Logger("voltTrigger_redraw : redraw %d\n",on);
     DSODisplay::drawTrigger( DSOCapture::getTriggerModeAsText(),on);
 }
 /**
  * 
  * @param inc
  */
 void voltTrigger_incdec(int inc)
 {     
    int v=(int)DSOCapture::getTriggerMode();
    v+=inc;
    int mod=1+(int)(DSOCapture::Trigger_Run);
    while(v<0)
    {
       v+=mod;
    }
    v%=mod;
    DSOCapture::stopCapture();
    DSOCapture::setTriggerMode((DSOCapture::TriggerMode)v);
    DSOCapture::startCapture(240);
    Logger("voltTrigger_incdec : %d\n",inc);
 }
 //-------
 // Volt / Offset
 void voltTriggerValue_redraw(bool on)
 {
    float conv=DSOCapture::getVoltToPix();
    float  v=DSOCapture::getTriggerVoltage();
    DSODisplay::drawVoltageTrigger(on, v*conv);
    DSODisplay::printTriggerValue( v,on);
    Logger("voltTriggerValue_redraw : redraw %d\n",on);     
 }
 /**
  * 
  * @param inc
  */
 void voltTriggerValue_incdec(int inc)
 {
     Logger("voltTriggerValue_incdec : %d\n",inc);
     if(inc)
     {
        float conv=DSOCapture::getVoltToPix();
        float  v=(DSOCapture::getTriggerVoltage()+voltageOffset);
        DSODisplay::drawVoltageTrigger(false, v*conv);
        v+=(float)inc/conv;
        
        DSOCapture::stopCapture();
        DSOCapture::setTriggerVoltage(v);
        DSOCapture::startCapture(240);
     }
 }
 /**
  * 
  * @param on
  */
 void time_redraw(bool on)
 {     
     Logger("time redraw : redraw %d\n",on);
     DSODisplay::drawTime(DSOCapture::getTimeBaseAsText(),on);     
 }
 /**
  * 
  * @param inc
  */
 void time_incdec(int inc)
 {
    Logger("time : %d\n",inc);
     
    int ctime=DSOCapture::getTimeBase();
    ctime=(ctime+inc+DSO_NB_TIMEBASE)%DSO_NB_TIMEBASE;
    DSOCapture::stopCapture();
    DSOCapture::setTimeBase((DSOCapture::DSO_TIME_BASE )ctime);
    DSOCapture::startCapture(240);
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
     NULL,          // 0: up
     NULL,          // 1: down
     NULL,          // 2: rotary     
     &voltMenu,     // 3: DSO_BUTTON_VOLTAGE,
     &timeMenu,     // 4: DSO_BUTTON_TIME,
     &triggerMenu,  // 5: DSO_BUTTON_TRIGGER,
     NULL,          // 6: DSO_BUTTON_OK
     NULL,          // 7: xx
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
     // Secondary menu
     triggerValueMenu.redraw(false);
     voltOffset.redraw(false);
 }
 
 /**
  * 
  */
 void processStartStop()
 {
     // toggle start /stop
    switch(DSOCapture::state())
    {
        case DSOCapture:: CAPTURE_STOPPED:        
            DSOCapture::startCapture(240);
            break;
        case DSOCapture:: CAPTURE_RUNNING:                                    
        case DSOCapture:: CAPTURE_DONE:
            DSOCapture::stopCapture();
            break;

    } 
 }
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
            debug("Event:%d , key:%d\n",kind,key);
            switch(kind)
            {
                case EVENT_SHORT_PRESS:
                    { // different key ?
                        
                        if(key==DSOControl::DSO_BUTTON_ROTARY)
                        {
                            processStartStop();
                            return;
                        }
                        
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
                break;
                case EVENT_LONG_PRESS:
                    { 
                        switch(key)
                        {
                            case DSOControl::DSO_BUTTON_ROTARY:
                            {
                                DSOCapture::stopCapture();
                                menuManagement(control);
                                redrawEverything();
                                control->saveSettings();
                                currentMenu=NULL;
                                initUiEvent();
                                DSOCapture::startCapture(240);
                                break;
                            }
                            case DSOControl::DSO_BUTTON_VOLTAGE:
                            {
                                DSOCapture::stopCapture();
                                autoSetup();
                                redrawEverything();                                
                                currentMenu=NULL;
                                initUiEvent();                                
                                DSOCapture::startCapture(240);
                            }
                                break;
                            default:
                               Logger("Unhandled ui key long press\n");
                               break;
                        }
                    }
                break;
                default:
                    Logger("Unhandled ui key\n");
                    break;
            }
     }
     int incdec=control->getRotaryValue();
     debug("Rotary: %d\n",incdec);
     if(incdec && currentMenu)   
     {
         currentMenu->incdec(incdec);     
         currentMenu->redraw(true);
     }
 }
