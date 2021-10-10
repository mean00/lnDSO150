
#include "lnArduino.h"
#include "dso_control.h"
#include "dso_menuEngine.h"
#include "dso_gfx.h"
extern DSOControl *control;

     

int currentFQ=1;

#define MAKEFQITEM(name,val) static const MenuListItem name={&currentFQ,val};

MAKEFQITEM(fq100,1)
MAKEFQITEM(fq1k,2)
MAKEFQITEM(fq10k,3)
MAKEFQITEM(fq100k,4)
MAKEFQITEM(fq1M,5)

#define FQ_MENU(x,y)     {MenuItem::MENU_INDEX, x,&y},    

static const MenuItem  fqMenu[]=
{
    {MenuItem::MENU_TITLE, "Frequency",NULL},
    FQ_MENU("100 Hz" ,  fq100)
    FQ_MENU("1 kHz",    fq1k)
    FQ_MENU("10 kHz",   fq10k)
    FQ_MENU("100 kHz",  fq100k)
    FQ_MENU("1 Mhz",    fq1M)
    {MenuItem::MENU_END, NULL,NULL}
};
#define RANGE_MENU(x,y)     {MenuItem::MENU_TITLE, x,NULL},     
static const MenuItem  amplitudeMenu[]=
{
    {MenuItem::MENU_TITLE, "Range",NULL},
    RANGE_MENU("3.3v" ,1)
    RANGE_MENU("100mv" ,0)
    {MenuItem::MENU_END, NULL,NULL}
};
static const MenuItem  signalMenu[]=
{
    {MenuItem::MENU_TITLE, "Test Signal",NULL},
    {MenuItem::MENU_SUBMENU, "Range",(const void *)&amplitudeMenu},
    {MenuItem::MENU_SUBMENU, "Frequency",(const void *)&fqMenu},
    {MenuItem::MENU_END, NULL,NULL}
};
static const MenuItem  calibrationMenu[]=
{
    {MenuItem::MENU_TITLE, "Calibration",NULL},
    {MenuItem::MENU_TITLE, "Basic Calibrate",NULL},    
    {MenuItem::MENU_TITLE, "Fine Calibrate",NULL},
    {MenuItem::MENU_TITLE, "Wipe Calibration",NULL},
    {MenuItem::MENU_END, NULL,NULL}
};
bool toggle=true;

static const MenuItem  topMenu[]={
    {MenuItem::MENU_TITLE, "Main Menu",NULL},
    {MenuItem::MENU_SUBMENU, "Test signal",(const void *)&signalMenu},
    {MenuItem::MENU_TITLE, "Button Test",NULL},
    {MenuItem::MENU_TOGGLE, "toggle",&toggle},
    {MenuItem::MENU_SUBMENU, "Calib ration",(const void *)&calibrationMenu},
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
    DSO_GFX::clear(0x1f);
    while(1)
    {
        xDelay(100);
    }
}
// EOF
