
#include "dso_global.h"
#include "dso_gfx.h"

#define FONT_SIZE 12
/**
 * 
 * @param array
 */
static void printxy(int x, int y, const char *t)
{
    tft->setCursor(x, y);
    tft->myDrawString(t);
}

/**
 * 
 * @param title
 */
void DSO_GFX::newPage(const char *title)
{
    tft->fillScreen(BLACK);  
    tft->fillRect(0,0,320,FONT_SIZE+4,WHITE);
    int l=strlen(title)*FONT_SIZE;
    int center=(320-l)/2;
    tft->setTextColor(BLACK,WHITE);
    printxy(center,2,title);
    tft->setTextColor(WHITE,BLACK);
}