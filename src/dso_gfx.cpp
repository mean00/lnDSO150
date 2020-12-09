
#include "dso_global.h"
#include "dso_gfx.h"

#define FONT_SIZE_X 12
#define FONT_SIZE_Y 16
/**
 * 
 * @param array
 */
void DSO_GFX::markup(const char *t)
{
    bool inverted=false;    
    while(1)
    {
        const char *next=strstr(t,"@");
        if(!next)
        {
             tft->myDrawString(t);             
             return;             
        }
        // Else just print the part till the @
        if(next!=t)
            tft->myDrawStringN(t,(int)(next-t));
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
 * @param array
 */
void DSO_GFX::printxy(int x, int y, const char *t)
{
    tft->setCursor(x, y);
    markup(t);
}
/**
 * 
 * @param title
 */
void DSO_GFX::newPage(const char *title)
{
    tft->fillScreen(BLACK);  
    tft->fillRect(0,0,320,FONT_SIZE_Y+4,WHITE);    
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
    center(title,FONT_SIZE_X*(2+1));    
}


/**
 * 
 * @param title
 */
void DSO_GFX::bottomLine(const char *title)
{
    center(title,240-2*FONT_SIZE_Y);
}
