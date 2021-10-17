/**
 * This is a little helper class to make menus easier
 * Just a wrapper on top of ili
 */
#include "lnArduino.h"
#include "dso_gfx.h"
#include "simpler9341.h"
#include "DSO_portBArbitrer.h"

#define FONT_SIZE_X 13
#define FONT_SIZE_Y 28

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
void DSO_GFX_markup(const char *t)
{
    bool inverted=false;    
    while(1)
    {
        const char *next=strstr(t,"@");
        if(!next)
        {
            tft->print(t);
            tft->setTextColor(WHITE,BLACK);
            return;             
        }
        // Else just print the part till the @
        if(next!=t)
        {
            const char *c=t;
            for(;c<next;c++)
                tft->writeChar(*c);
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
void DSO_GFX_center(const char *p,int y)
{
    int colx,coly;
    colx=(320-strlen(p)*FONT_SIZE_X)/2;
    if(y<0) coly=240+y*FONT_SIZE_Y;
    else coly=y*FONT_SIZE_Y;

    tft->setCursor(colx,coly);
    DSO_GFX_markup(p);
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
 * @param big
 */
void DSO_GFX::setBigFont(bool big)
{
    if(big)
        tft->setFontSize(ili9341::BigFont);
    else
        tft->setFontSize(ili9341::MediumFont);
}

/**
 * 
 * @param array
 */
void DSO_GFX::printxy(int x, int y, const char *t)
{
    AutoLcd autolcd;
    int l,w;
    if(y<0) l=240+FONT_SIZE_Y*y;
    else l=FONT_SIZE_Y*y+FONT_SIZE_Y;
    
    if(x<0) w=320+FONT_SIZE_X*x;
    else w=FONT_SIZE_X*x;
    
    tft->setCursor(w, l+4);
    DSO_GFX_markup(t);
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
 * @param color
 */
void DSO_GFX::clearBody(int color)
{
      AutoLcd autolcd;
      tft->square(color,
            0, FONT_SIZE_Y+8,
            320,240-FONT_SIZE_Y*3-8);
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
    DSO_GFX_center(title,1);    
    tft->setTextColor(WHITE,BLACK);
}


/**
 * 
 * @param title
 */
void DSO_GFX::subtitle(const char *title)
{
    AutoLcd autolcd;
    DSO_GFX_center(title,FONT_SIZE_X*(2+1));    
}


/**
 * 
 * @param title
 */
void DSO_GFX::bottomLine(const char *title)
{
    AutoLcd autolcd;
    DSO_GFX_center(title,-1);
}
/**
 * 
 * @param text
 */
void DSO_GFX::printMenuTitle(const char *text)
{
    AutoLcd autolcd;
    int fg,bg;
    tft->getTextColor(fg,bg);
    DSO_GFX::setTextColor(BLACK,BLUE); 
    tft->square(BLUE,0,0+8,320,FONT_SIZE_Y);
    DSO_GFX_center(text,1);    
    DSO_GFX::setTextColor(fg,bg); 
}
/**
 * 
 * @param text
 * @param line
 */
void DSO_GFX::center(const char *text, int line)
{
    DSO_GFX_center(text,line);
}
/**
 * 
 * @param t
 */
void DSO_GFX::markup(const char *t)
{
    DSO_GFX_markup(t);
}

// EOF