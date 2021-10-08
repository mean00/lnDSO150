
#include "lnArduino.h"
#include "dso_control.h"
#include "dso_menuEngine.h"
extern DSOControl *control;

     
#define FQ_MENU(x,y)     {MenuItem::MENU_TITLE, x,NULL},     
static const MenuItem  fqMenu[]=
{
    {MenuItem::MENU_TITLE, "Frequency",NULL},
    FQ_MENU("100 Hz" ,100)
    FQ_MENU("1 kHz",  1000)
    FQ_MENU("10 kHz", 10000)
    FQ_MENU("100 kHz",100000)
    {MenuItem::MENU_BACK, "Back",NULL},
    {MenuItem::MENU_END, NULL,NULL}
};
#define RANGE_MENU(x,y)     {MenuItem::MENU_TITLE, x,NULL},     
static const MenuItem  amplitudeMenu[]=
{
    {MenuItem::MENU_TITLE, "Range",NULL},
    RANGE_MENU("3.3v" ,1)
    RANGE_MENU("100mv" ,0)
    {MenuItem::MENU_BACK, "Back",NULL},
    {MenuItem::MENU_END, NULL,NULL}
};
static const MenuItem  signalMenu[]=
{
    {MenuItem::MENU_TITLE, "Test Signal",NULL},
    {MenuItem::MENU_SUBMENU, "Range",(const void *)&amplitudeMenu},
    {MenuItem::MENU_SUBMENU, "Frequency",(const void *)&fqMenu},
    {MenuItem::MENU_BACK, "Back",NULL},
    {MenuItem::MENU_END, NULL,NULL}
};
static const MenuItem  calibrationMenu[]=
{
    {MenuItem::MENU_TITLE, "Calibration",NULL},
    {MenuItem::MENU_TITLE, "Basic Calibrate",NULL},    
    {MenuItem::MENU_TITLE, "Fine Calibrate",NULL},
    {MenuItem::MENU_TITLE, "Wipe Calibration",NULL},
    {MenuItem::MENU_BACK, "Back",NULL},
    {MenuItem::MENU_END, NULL,NULL}
};
static const MenuItem  topMenu[]={
    {MenuItem::MENU_TITLE, "Main Menu",NULL},
    {MenuItem::MENU_SUBMENU, "Test signal",(const void *)&signalMenu},
    {MenuItem::MENU_TITLE, "Button Test",NULL},
    {MenuItem::MENU_SUBMENU, "Calibration",(const void *)&calibrationMenu},
    {MenuItem::MENU_BACK, "Back",NULL},
    {MenuItem::MENU_END, NULL,NULL}
};


/**
 * 
 */
void testMenu()
{    
    const MenuItem *tem=topMenu;
    MenuManager man(control, tem);
    man.run();
    while(1)
    {
        xDelay(100);
    }
}
// EOF
