
#pragma once

#include "Adafruit_GFX_AS.h" //
#include "Adafruit_ILI9341_STM.h" //
#include "gfxfont.h"
/**
 */
class ILI9341 : public Adafruit_ILI9341_STM
{
public:  
  typedef enum FontSize
  {
    SmallFont,MediumFont,BigFont
  };
  
        ILI9341(int8_t _CS, int8_t _DC, int8_t _RST = -1);
        ~ILI9341();
        
        //--
        
        void  setFontFamily(const GFXfont *small, const GFXfont *medium, const GFXfont *big);
        void  myDrawString(const char *st, int padd_up_to_n_pixels=0);
        void  setFontSize(FontSize size);
        //
        void drawBitmap(int width, int height, int wx, int wy, int fgcolor, int bgcolor, const uint8_t *data);
        void drawRLEBitmap(int width, int height, int wx, int wy, int fgcolor, int bgcolor, const uint8_t *data);
        
public:        
        class FontInfo
        {
        public:
          int               maxHeight;          
          int               maxWidth;
          uint16_t         *filler;
          const GFXfont    *font;        
        };
protected:        
        FontInfo          fontInfo[3];
        int               myDrawChar(int x, int y, unsigned char c,  int color, int bg,FontInfo &info);
        int               mySquare(int x, int y, int w, int top, uint16_t *filler);
        FontInfo          *currentFont;
        
};
