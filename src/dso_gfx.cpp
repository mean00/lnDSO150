/**
 * This is a little helper class to make menus easier
 * Just a wrapper on top of ili
 */
#include "dso_global.h"
#include "dso_gfx.h"
#include "simpler9341.h"
#include "DSO_portBArbitrer.h"
#define FONT_SIZE_X 12
#define FONT_SIZE_Y 16

static ili9341 *tft=NULL;
extern DSO_portArbitrer *arbitrer;

void DSO_GFX::init(ili9341 *i)
{
    tft=i;
}
class AutoLcd
{
public:
    AutoLcd()
    {
        arbitrer->beginLCD();
    }
    ~AutoLcd()
    {
        arbitrer->endLCD();
    }
};
/**
 * 
 * @param array
 */
void DSO_GFX::markup(const char *t)
{
    AutoLcd autolcd;
    
    bool inverted=false;    
    while(1)
    {
        const char *next=strstr(t,"@");
        if(!next)
        {
             tft->print(t);             
             return;             
        }
        // Else just print the part till the @
        if(next!=t)
        {
#warning FIXME            
            //tft->myDrawStringN(t,(int)(next-t));
            tft->print(t);
        }
        t=next+1;
        inverted=!inverted;
        if(inverted)
             tft->setTextColor(BLACK,WHITE);
        else
             tft->setTextColor(WHITE,BLACK);
    }    
}
/**
 * 
 * @param p
 * @param y
 */
void DSO_GFX::center(const char *p,int y)
{
    int l=strlen(p)*FONT_SIZE_X;
    int xcenter=(320-l)/2;    
    if(xcenter<0) xcenter=0;
    tft->setCursor(xcenter, y);
    markup(p);
}
/**
 * 
 * @param fg
 * @param bg
 */
void DSO_GFX::setTextColor(int fg,int bg)
{
    tft->setTextColor(fg,bg);
}
/**
 * 
 * @param array
 */
void DSO_GFX::printxy(int x, int y, const char *t)
{
    AutoLcd autolcd;
    
    tft->setCursor(x, y);
    markup(t);
}
/**
 * 
 * @param color
 */
void DSO_GFX::clear(int color)
{
    AutoLcd autolcd;
    tft->fillScreen(color);
}

/**
 * 
 * @param title
 */
void DSO_GFX::newPage(const char *title)
{
    AutoLcd autolcd;
    tft->fillScreen(BLACK);  
    tft->square(WHITE,0,0,320,FONT_SIZE_Y+4);
    tft->setTextColor(BLACK,WHITE);
    center(title,2);    
    tft->setTextColor(WHITE,BLACK);
}


/**
 * 
 * @param title
 */
void DSO_GFX::subtitle(const char *title)
{
    AutoLcd autolcd;
    center(title,FONT_SIZE_X*(2+1));    
}


/**
 * 
 * @param title
 */
void DSO_GFX::bottomLine(const char *title)
{
    AutoLcd autolcd;
    center(title,240-2*FONT_SIZE_Y);
}
// EOF