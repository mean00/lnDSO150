/***************************************************
 STM32 duino based firmware for DSO SHELL/150
 *  * GPL v2
 * (c) mean 2019 fixounet@free.fr
 ****************************************************/

#include "dso_includes.h"
#include "dso_menu.h"
#include "dso_global.h"
#include "dso_calibrate.h"
extern testSignal *myTestSignal;
void updateFrequency(int fq)
{
    myTestSignal->setFrequency(fq);
}
#define MKFQ(x,y) void fq##x() {updateFrequency(y); }

MKFQ(100,100)
MKFQ(1000,1000)
MKFQ(10000,10000)
MKFQ(100000,100000)
        
#define FQ_MENU(x,y)     {MenuItem::MENU_CALL, x,(void *)fq##y},     
const MenuItem  fqMenu[]=
{
    {MenuItem::MENU_TITLE, "Frequency",NULL},
    FQ_MENU("100 Hz" ,100)
    FQ_MENU("1 kHz",  1000)
    FQ_MENU("10 kHz", 10000)
    FQ_MENU("100 kHz",100000)
    {MenuItem::MENU_BACK, "Back",NULL},
    {MenuItem::MENU_END, NULL,NULL}
};

const MenuItem  signalMenu[]=
{
    {MenuItem::MENU_TITLE, "Test Signal",NULL},
    {MenuItem::MENU_TOGGLE, "3v signal",NULL},
    {MenuItem::MENU_SUBMENU, "Frequency",(const void *)&fqMenu},
    {MenuItem::MENU_BACK, "Back",NULL},
    {MenuItem::MENU_END, NULL,NULL}
};
const MenuItem  topMenu[]={
    {MenuItem::MENU_TITLE, "Main Menu",NULL},
    {MenuItem::MENU_SUBMENU, "Test signal",(const void *)&signalMenu},
    {MenuItem::MENU_CALL, "Calibration",(const void *)DSOCalibrate::calibrate},
    {MenuItem::MENU_BACK, "Back",NULL},
    {MenuItem::MENU_END, NULL,NULL}
};


/**
 */
void  menuManagement(void)
{
     tft->fillScreen(BLACK);
     const MenuItem *tem=topMenu;
     MenuManager man(tem);
     man.run();
}
/**
 * 
 * @param menu
 */
MenuManager::MenuManager(const MenuItem *menu)
{
    _menu=menu;
}
/**
 * 
 */
MenuManager::~MenuManager()
{
    _menu=NULL;
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
    tft->setCursor(100,40+24*line);
    if(onoff)
        tft->setTextColor(BLACK,BG_COLOR); 
    else  
        tft->setTextColor(BG_COLOR,BLACK);
    tft->myDrawString(text);
}
void MenuManager::printMenuTitle(const char *text)
{
    tft->setTextColor(GREEN,BLACK); 
    tft->setCursor(80,40-24);
    tft->myDrawString(text);
}
/**
 * 
 */
void MenuManager::run(void)
{
     tft->fillScreen(BLACK);
     tft->setFontSize(Adafruit_TFTLCD_8bit_STM32::BigFont);
     runOne(_menu);

};
/**
 */
void MenuManager::redraw(const char *title, int n,const MenuItem *xtop, int current)
{
    tft->fillScreen(BLACK);
    printMenuTitle(title); 
    for(int i=0;i<n;i++)
    {
        printMenuEntry(current==i,i,xtop[i].menuText);
    }        
}
/**
 */
void MenuManager::runOne( const MenuItem *xtop)
{
     tft->fillScreen(BLACK);
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
     while(1)
     {
next:         
        redraw(title,n,xtop,current);
        while(1)
        {
                  if(controlButtons->getButtonEvents(DSOControl::DSO_BUTTON_OK) & EVENT_LONG_PRESS)
                    return;
                  if(controlButtons->getButtonEvents(DSOControl::DSO_BUTTON_OK) & EVENT_SHORT_PRESS)
                  {
                      switch(xtop[current].type)
                      {
                      case MenuItem::MENU_BACK: return; break;
                      case MenuItem::MENU_KEY_VALUE:  break;
                      case MenuItem::MENU_SUBMENU: 
                            {
                                const MenuItem *sub=(const MenuItem *)xtop[current].cookie;
                                runOne(sub);
                                goto next;
                            }
                            break;
                      case MenuItem::MENU_CALL: 
                            {
                                typedef void cb(void);
                                cb *c=(cb *)xtop[current].cookie;
                                c();
                                goto next;
                            }
                          break;
                      case MenuItem::MENU_END: return;break;
                      case MenuItem::MENU_TOGGLE: break;
                      default: xAssert(0);break;
                      }
                      break;
                  }
                  int inc=controlButtons->getRotaryValue();
                  if(inc)
                  {
                    current+=inc;
                    while(current<0) current+=n;
                    while(current>n) current-=n;
                    break;
                  }
        }
     }
};
// EOF