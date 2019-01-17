/**
 *  \brief Extended ILI9341 driver with
 *    * Support for adafruit "truetype" fonts
 *    * Mono bitmap draw (compressed or not)
 * 
 */
#include "Adafruit_TFTLCD_8bit_STM32.h"
#include "Adafruit_TFTLCD_8bit_STM32_priv.h"


/**
 * \fn mySquare
 * \brief Draw a square of w*xheight size at position x,y
 * \param filler is a prefill color array
 */
int Adafruit_TFTLCD_8bit_STM32::mySquare(int x, int y, int w, int xheight, uint16_t filler)
{
    if(w+x>=width())
    {
        w=width()-x;
        if(w<=0) return 0;
    }
    if(xheight+y>=height())
    {
        xheight=height()-y;
        if(xheight<=0)
            return 0;
    }
    setAddrWindow(x,y,                  x+w-1, y+xheight);
    flood(filler,w*xheight);
    return 0;
}

/**
 * 
 * @param x
 * @param y
 * @param c
 * @param color
 * @param bg
 * @return 
 */
#if 0
    #define debug(x) x
#else
    #define debug(x) {}
#endif

int Adafruit_TFTLCD_8bit_STM32::myDrawChar(int x, int y, unsigned char c,  int color, int bg,FontInfo &infos)
{
    c -= infos.font->first;
    GFXglyph *glyph  = infos.font->glyph+c;
    
    uint8_t *p= infos.font->bitmap+glyph->bitmapOffset;        
    int  w  = glyph->width;
    int  h  = glyph->height;    
    uint16_t column[TFTWIDTH];
    
    debug(uint16_t oldbg=bg);
    debug(bg=YELLOW);
    debug(color=GREEN);
    
    // Special case, space, it has yOffsset > 0
    if(infos.font->first+c==' ')
    {
        int adv=glyph->xAdvance;
        int top=infos.maxHeight;
         mySquare(x,y-top,
                  infos.maxWidth,
                  top+2,bg);
         return adv;
    }
    
    // top & bottom
    int top=infos.maxHeight+glyph->yOffset;
    mySquare(x,y-infos.maxHeight,glyph->xAdvance,top,bg);

    int bottom=-glyph->yOffset-h;
    mySquare(x,y-bottom,glyph->xAdvance,bottom+2,bg);      
    

    y+= glyph->yOffset;   // offset is <0 most of the time
    
    int left=glyph->xOffset;
    int right=glyph->xAdvance-(w+left);
    if(right<0) right=0;
       
    int    finalColor;    
    int  bits = 0, bit = 0;
    setAddrWindow(x,y, x+glyph->xAdvance-1, y+h);
    debug(bg=oldbg);
    uint16_t *col=column;
    
    // Pre-fill & left /right
     for(int i=0;i<left;i++)
            column[i]=bg;
     for(int i=0;i<right;i++) 
            column[left+w+i]=bg;    
    // fill in body
    bool first=true;
    for( int line=h-1;line>=0;line--)
    {      
        col=column+left;     
        // mid
        for( int xcol=w-1;xcol>=0;xcol--)
        {
            if(!bit) // reload ?
            {
                bits= *p++;
                bit = 0x80;
                if(xcol>=8) // speed up some special cases
                {
                    switch(bits)
                    {
                        
                        case 0xff: 
                                    *col++=color; *col++=color;*col++=color; *col++=color;
                                    *col++=color; *col++=color;*col++=color; *col++=color;
                                    xcol-=7;
                                    bit=0;
                                    continue;
                                    break;
                        case 0x00: 
                                    *col++=bg;*col++=bg;*col++=bg;*col++=bg;
                                    *col++=bg;*col++=bg;*col++=bg;*col++=bg;
                                    xcol-=7;
                                    bit=0;
                                    continue;
                        case 0x7f: 
                                    *col++=bg; *col++=color;
                                    *col++=color; *col++=color;
                                    *col++=color; *col++=color;
                                    *col++=color; *col++=color;
                                    xcol-=7;
                                    bit=0;
                                    continue;
                                                            
                                    break;
                        
                        default:break;
                    }
                }
            }            
            if(bits & bit) 
                finalColor=color;  
            else
                finalColor=bg;
            *(col++)=finalColor;            
            bit=bit>>1;
        }
        // 9ms here
        pushColors(column,glyph->xAdvance,first);
        first=false;
    }   
    return glyph->xAdvance;
}

// EOF