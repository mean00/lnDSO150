/**
 *  \brief Extended ILI9341 driver with
 *    * Support for adafruit "truetype" fonts
 *    * Mono bitmap draw (compressed or not)
 * 
 */
#include "Adafruit_TFTLCD_8bit_STM32.h"
#include "Adafruit_TFTLCD_8bit_STM32_priv.h"

#define pgm_read_dword(addr) (*(const unsigned long *)(addr))
#define pgm_read_byte(addr) (*(const unsigned char *)(addr))
#define pgm_read_pointer(addr) ((void *)pgm_read_dword(addr))

/**
 * 
 * @param st
 */
 void  Adafruit_TFTLCD_8bit_STM32::myDrawString(const char *st,int padd_up_to_n_pixels)
 {
     if(!currentFont)
         return;
     int l=strlen(st);
   
     int lastColumn=0;
     
     int endX=cursor_x+padd_up_to_n_pixels;
     
     for(int i=0;i<l;i++)
     {
         int of=myDrawChar(cursor_x,cursor_y+currentFont->maxHeight,
                           st[i],
                           textcolor,textbgcolor,*currentFont);
         cursor_x+=of;
         if(cursor_x>=_width) return;
     }
     int leftOver=endX-cursor_x;
     if(leftOver>0)
     {
         while(leftOver>0)
         {
             int rnd=leftOver;
            if(rnd>currentFont->maxWidth) rnd=currentFont->maxWidth;
            mySquare(cursor_x,cursor_y,
                  rnd, currentFont->maxHeight+2, textbgcolor);
            cursor_x+=rnd;
            leftOver=endX-cursor_x;
            
         }
     }
 }

/**
 * 
 * @param size
 */
void  Adafruit_TFTLCD_8bit_STM32::setFontSize(FontSize size)
{
    switch(size)
    {
        case SmallFont :  currentFont=fontInfo+0;break;
        default:
        case MediumFont :   currentFont=fontInfo+1;break;
        case BigFont :   currentFont=fontInfo+2;break;
    }    
}
/**
 * \fn checkFont
 * \brief extract max width/ max height from the font
 */
static void checkFont(const GFXfont *font, Adafruit_TFTLCD_8bit_STM32::FontInfo *info)
{
    int mW=0,mH=0;
    int x,y;
   for(int i=font->first;i<font->last;i++)
   {
         GFXglyph *glyph  = font->glyph+i-font->first;
         x=glyph->xAdvance;
         y=-glyph->yOffset;
         if(x>mW) mW=x;         
         if(y>mH) mH=y;
   }
    info->maxHeight=mH + 1;
    info->maxWidth=mW;    
    info->font=font;
}

/**
 * 
 * @param small
 * @param medium
 * @param big
 */
void  Adafruit_TFTLCD_8bit_STM32::setFontFamily(const GFXfont *small, const GFXfont *medium, const GFXfont *big)
{
    checkFont(small, fontInfo+0);
    checkFont(medium,fontInfo+1);
    checkFont(big,   fontInfo+2);
}



/**
 */
void Adafruit_TFTLCD_8bit_STM32::drawBitmap(int width, int height, int wx, int wy, int fgcolor, int bgcolor, const uint8_t *data)
{
    uint8_t *p=(uint8_t *)data;    
    uint16_t line[320];
    
    width>>=3;
    for(int y=0;y<height;y++)
    {
        uint16_t *o=line;
        setAddrWindow(wx, wy+y, wx+width*8-1, wy+y-1);
        for(int x=0;x<width;x++)
        {
            int stack=*p++;
            for(int step=0;step<8;step++)
            {
                int color;
                if(stack&0x80)                                        
                    color=fgcolor;
                else
                    color=bgcolor;
                *o++=color;
                stack<<=1;
            }            
        }    
        pushColors(line,width*8,0);
    }   
}

/**
 */
void Adafruit_TFTLCD_8bit_STM32::drawRLEBitmap(int widthInPixel, int height, int wx, int wy, int fgcolor, int bgcolor, const uint8_t *data)
{
    uint8_t *p=(uint8_t *)data;    
    uint16_t line[320];
    bool first=true;
    
    int widthInByte=(widthInPixel>>3); // pixel -> bytes
    for(int y=0;y<height;y++)
    {
        uint16_t *o=line;
        setAddrWindow(wx, wy+y, wx+widthInPixel, wy+y);
        for(int x=0;x<widthInByte;) // in bytes
        {
            int val=*p++;
            int count=1;
            if(val==0x76)
            {
                val=*p++;
                count=*p++;
            }
            for(int i=0;i<count;i++)
            {
                int stack=val;
                for(int step=0;step<8;step++)
                {
                    int color;
                    if(stack&0x80)                                        
                        color=fgcolor;
                    else
                        color=bgcolor;
                    *o++=color;
                    stack<<=1;
                }            
            }
            x+=count;
        }    
        pushColors(line,widthInPixel,first);
        //first=false;
    }   
}


// EOF