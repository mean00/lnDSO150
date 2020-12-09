/***************************************************
 STM32 duino based firmware for DSO SHELL/150
 *  * GPL v2
 * (c) mean 2019 fixounet@free.fr
 ****************************************************/

#include "dso_includes.h"
#include "dso_menuEngine.h"
#include "dso_global.h"
#include "dso_calibrate.h"
extern testSignal *myTestSignal;
extern void buttonTest(void);

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
const MenuItem  calibrationMenu[]=
{
    {MenuItem::MENU_TITLE, "Calbration",NULL},
    {MenuItem::MENU_CALL, "Basic Calibrate",(const void *)DSOCalibrate::zeroCalibrate},    
    {MenuItem::MENU_CALL, "Fine Calibrate",(const void *)DSOCalibrate::voltageCalibrate},
    {MenuItem::MENU_CALL, "Wipe Calibration",(const void *)DSOCalibrate::decalibrate},
    {MenuItem::MENU_BACK, "Back",NULL},
    {MenuItem::MENU_END, NULL,NULL}
};
const MenuItem  topMenu[]={
    {MenuItem::MENU_TITLE, "Main Menu",NULL},
    {MenuItem::MENU_SUBMENU, "Test signal",(const void *)&signalMenu},
    {MenuItem::MENU_CALL, "Button Test",(const void *)buttonTest},
    {MenuItem::MENU_SUBMENU, "Calibration",(const void *)&calibrationMenu},
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
// EOF