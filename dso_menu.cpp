/***************************************************
 STM32 duino based firmware for DSO SHELL/150
 *  * GPL v2
 * (c) mean 2019 fixounet@free.fr
 ****************************************************/

#include "dso_includes.h"
#include "dso_menu.h"
#include "dso_global.h"

const MenuItem  signalMenu[]=
{
    {MenuItem::MENU_TOGGLE, "3v signal",NULL},
    {MenuItem::MENU_CALL, "Frequency",NULL},
    {MenuItem::MENU_END, NULL,NULL}
};
const MenuItem  topMenu[]={
    {MenuItem::MENU_SUBMENU, "Test signal",(const void *)&signalMenu},
    {MenuItem::MENU_END, NULL,NULL}
};


/**
 */
void  menuManagement(void)
{
     tft->fillScreen(BLACK);
     const MenuItem *tem=topMenu;
     MenuManager man("Main menu",tem);
     man.run();
}
/**
 * 
 * @param menu
 */
MenuManager::MenuManager(const char *title,const MenuItem *menu)
{
    _menu=menu;
    _title=title;
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
    tft->setCursor(100,100+20*line);
    if(onoff)
        tft->setTextColor(BLACK,BG_COLOR); 
    else  
        tft->setTextColor(BG_COLOR,BLACK);
    tft->myDrawString(text);
}
/**
 * 
 */
void MenuManager::run(void)
{
     tft->fillScreen(BLACK);
     
     int n=0;
     {
        const MenuItem  *top=_menu;
        while(top->type!=MenuItem::MENU_END)
        {
            top++;
            n++;
        }
     }
     printMenuEntry(false,0,_title);
     printMenuEntry(false,n+1,"Back");
     // draw them 
     int current=0;
     while(1)
     {
        for(int i=0;i<n;i++)
        {
            printMenuEntry(current==i,i+1,_menu[i].menuText);
        }
     }
     while(1)
     {
         
     };
};
