/***************************************************
 * This is a very basic settings menu helper class
 * It is designed to be dwelling in flash, not in ram
 ****************************************************/
#include "lnArduino.h"
#include "dso_gfx.h"
#include "dso_menuEngine.h"
#include "dso_control.h"

static MenuManager *_instance;
#define USE_MENU_BUTTON DSOControl::DSO_BUTTON_ROTARY
//#define USE_MENU_BUTTON DSOControl::DSO_BUTTON_OK

/**
 * 
 * @param menu
 */
MenuManager::MenuManager(DSOControl *ctl, const MenuItem *menu)
{
    _menu=menu;
    _instance=this;
    _control=ctl;
}
/**
 * 
 */
MenuManager::~MenuManager()
{
    _menu=NULL;
    _instance=NULL;
    _control=NULL;
}
/**
 * 
 * @param onoff
 * @param line
 * @param text
 * @return 
 */
void MenuManager::printMenuEntry(bool onoff, int line,const char *text)
{
    #define BG_COLOR GREEN    
    if(onoff)
        DSO_GFX::setTextColor(BLACK,BG_COLOR); 
    else  
        DSO_GFX::setTextColor(BG_COLOR,BLACK);
    DSO_GFX::printxy(8,1+line,text);
}
void MenuManager::printPrefix(bool onoff, int line,const char *text)
{
    #define BG_COLOR GREEN    
    if(onoff)
        DSO_GFX::setTextColor(BLACK,BG_COLOR); 
    else  
        DSO_GFX::setTextColor(BG_COLOR,BLACK);
    DSO_GFX::printxy(6,1+line,text);
}

void MenuManager::printMenuTitle(const char *text)
{
    DSO_GFX::printMenuTitle(text); 
}

void MenuManager::printBackHint()
{
    DSO_GFX::setTextColor(BLACK,BLUE); 
    DSO_GFX::printxy(-5,-1,"Back");
    DSO_GFX::setTextColor(GREEN,BLACK); 
}
/**
 * 
 */
void MenuManager::run(void)
{
    DSO_GFX::clear(BLACK);
    DSO_GFX::setBigFont(true);   
    runOne(_menu);     
};
/**
 */
void MenuManager::redraw(const char *title, int n,const MenuItem *xtop, int current)
{
    DSO_GFX::clearBody(BLACK);
    printMenuTitle(title); 
    for(int i=0;i<n;i++)
    {
        bool mark=false;
        switch(xtop[i].type)
        {
            case MenuItem::MENU_TOGGLE :
                if(*(bool *) xtop[i].cookie) 
                    mark=true;
                break; 
            case MenuItem::MENU_INDEX:
            {
                MenuListItem *tem=(MenuListItem *)xtop[i].cookie;
                if(tem->thisItem==*(tem->item)) mark=true;
            }
                break;
            default:
                break;
        }
        if(mark)
            printPrefix(current==i,i,"v");
        printMenuEntry(current==i,i,xtop[i].menuText);
    }         
}

void MenuManager::blink(int current, const char *text)
{
    for(int i=0;i<5;i++)
    {
           printMenuEntry(false,current,text);
           xDelay(80);
           printMenuEntry(true,current,text);
           xDelay(80);
    }     
}
/**
 * 
 * @param e
 * @return 
 */
void MenuManager_controlEvent(DSOControl::DSOEvent e)
{
    xAssert(_instance);
    _instance->controlEvent();
}
/**
 */
void MenuManager::runOne( const MenuItem *xtop)
{
     DSO_GFX::clear(BLACK);
     printBackHint();
     DSOControl::ControlEventCb *oldCb=_control->getCb();
     Logger("Entering menu\n");
     
     // Wait for the key to be released
     while(1)
     {
         if(!_control->getButtonState(DSOControl::DSO_BUTTON_ROTARY)) break;
         xDelay(50);
     }
     _control->changeCb( MenuManager_controlEvent);     
     _control->purgeEvent();
     runOne_(xtop);
     _control->changeCb(oldCb);
     Logger("Exiting menu\n");
}
/**
 * 
 * @param evt
 */
void MenuManager::controlEvent()
{
    _sem.give();
}

/**
 * 
 * @param xtop
 */
void MenuManager::runOne_( const MenuItem *xtop)
{
     const char *title=xtop->menuText;
     xtop=xtop+1;
     int n=0;
     {
        const MenuItem  *top=xtop;
        while(top->type!=MenuItem::MENU_END)
        {
            top++;
            n++;
        }
     }
     // draw them 
     // 0 to N-1
     int current=0;    
next:            
        redraw(title,n,xtop,current);
        while(1)
        { 
                  _sem.take(200);
                  
                  int event;
                  while(event=_control->getQButtonEvent())
                  {
                    if(event==DSO_EVENT_Q(DSOControl::DSO_BUTTON_OK,EVENT_SHORT_PRESS))
                        return;
                    if(event==DSO_EVENT_Q(DSOControl::USE_MENU_BUTTON,EVENT_LONG_PRESS))
                        return;
                    if(event==DSO_EVENT_Q(DSOControl::USE_MENU_BUTTON,EVENT_SHORT_PRESS))                    
                    {
                        if(handlePress(title,n, xtop,current)) 
                        {
                            return;
                        }
                    }
                  }
                  int inc=_control->getRotaryValue();
                  if(inc)
                  {
                    current+=inc;
                    while(current<0) current+=n;
                    while(current>=n) current-=n;
                    goto next;
                  }     
        }
};
/**
 * It is just to keep runOne simple
 * @param xtop
 * @param current
 * @return 
 */
bool MenuManager::handlePress( const char *title,int n,const MenuItem *xtop,int current)
{
    switch(xtop[current].type)
    {
        case MenuItem::MENU_BACK: 
        case MenuItem::MENU_END:             
                return true; 
                break;
        case MenuItem::MENU_SUBMENU: 
              {
                  const MenuItem *sub=(const MenuItem *)xtop[current].cookie;
                  runOne_(sub);
                  redraw(title,n,xtop,current);
                  return false;
              }
              break;
        case MenuItem::MENU_CALL: 
              {
                  typedef void cb(void);
                  cb *c=(cb *)xtop[current].cookie;
                  blink(current,xtop[current].menuText);
                  c();
                  DSO_GFX::setBigFont(true);
                  return true;
              }
            break;
        case MenuItem::MENU_INDEX:
        {
            MenuListItem *item=(MenuListItem *)xtop[current].cookie;
            *(item->item)=item->thisItem;                          
            redraw(title,n,xtop,current);
            return false;
        }
        break;

        case MenuItem::MENU_TOGGLE: 
        {
            bool *e=(bool *)xtop[current].cookie;
            *e=!*e;
            redraw(title,n,xtop,current);
            return false;
        }
            break;
        case MenuItem::MENU_TITLE:
            return false;
            break;
        default: xAssert(0);break;
    }
    return false;
}

// EOF