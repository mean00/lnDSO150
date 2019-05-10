// Graphics library by ladyada/adafruit with init code from Rossum
// MIT license

#ifndef _ADAFRUIT_TFTLCD_8BIT_STM32_H_
#define _ADAFRUIT_TFTLCD_8BIT_STM32_H_

#include <Adafruit_GFX.h>

#include <libmaple/gpio.h>

/*****************************************************************************/
// LCD controller chip identifiers
typedef enum LCD_CHIP_ID
{
    ID_932X   =0,
    ID_7575    =1,
    ID_9341    =2,
    ID_HX8357D =3,
    ID_9338    =4,
    ID_ST7789  =5,
    ID_UNKNOWN =0xFF
};

/*****************************************************************************/
#define TFTWIDTH   240
#define TFTHEIGHT  320

// Initialization command tables for different LCD controllers
#define TFTLCD_DELAY 0xFF

// For compatibility with sketches written for older versions of library.
// Color function name was changed to 'color565' for parity with 2.2" LCD
// library.
#define Color565 color565

/*****************************************************************************/
#define	BLACK   0x0000
#define	BLUE    0x001F
#define	RED     0xF800
#define	GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

/*****************************************************************************/


/*****************************************************************************/
class Adafruit_TFTLCD_8bit_STM32 : public Adafruit_GFX 
{

 public:
 typedef enum FontSize
  {
    SmallFont,MediumFont,BigFont
  };
    class FontInfo
        {
        public:
          int               maxHeight;          
          int               maxWidth;
          uint16_t         *filler;
          const GFXfont    *font;        
        };
   
   static Adafruit_TFTLCD_8bit_STM32 *spawn(int id);
   
  //Adafruit_TFTLCD_8bit_STM32(uint8_t cs, uint8_t cd, uint8_t wr, uint8_t rd, uint8_t rst);
  Adafruit_TFTLCD_8bit_STM32(void);

  
          
          
  // Overrriden by derived classes          
  virtual void     begin()=0;
  virtual void     drawPixel(int16_t x, int16_t y, uint16_t color)=0;
  virtual void     floodPreamble()=0;
  virtual void     pushColorsPreamble()=0;
  virtual void     invertDisplay(bool i)=0;
  virtual void     setRotation(uint8_t x)=0;
  virtual void     setAddrWindow(int16_t x1, int16_t y1, int16_t x2, int16_t y2)=0;

  //-- common class
  
  void     drawFastHLine(int16_t x0, int16_t y0, int16_t w, uint16_t color);
  void     drawFastVLine(int16_t x0, int16_t y0, int16_t h, uint16_t color);
  
  void     drawFastHLineDotted(int16_t x0, int16_t y0, int16_t w, uint16_t color, uint16_t bg);
  void     drawFastVLineDotted(int16_t x0, int16_t y0, int16_t w, uint16_t color,uint16_t bg);
  
  void     fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t c);
  virtual void     fillScreen(uint16_t color)=0;
  static void     reset(void);
  void     setRegisters8(uint8_t *ptr, uint8_t n);
  void     setRegisters16(uint16_t *ptr, uint8_t n);

       // These methods are public in order for BMP examples to work:


  void     drawBitmap(int16_t x, int16_t y, int16_t w, int16_t h, const uint16_t * bitmap);
  static uint16_t readID(void);
/*****************************************************************************/
// Pass 8-bit (each) R,G,B, get back 16-bit packed color
// color coding on bits:
// high byte sill be sent first
// bit nr: 		15	14	13	12	11	 10	09	08		07	06	05	 04	03	02	01	00
// color/bit:	R5	R4	R3	R2	R1 | G5	G4	G3		G2	G1	G0 | B5	B4	B3	B2	B1
// 								R0=R5											B0=B5
/*****************************************************************************/
  uint16_t inline color565(uint8_t r, uint8_t g, uint8_t b) { return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3); }
  void     flood(uint16_t color, uint32_t len);
  void    flood2(uint16_t color, uint16_t bg,uint32_t len);
  int      mySquare(int x, int y, int w, int h, uint16_t filler);
 private:
   

  void     init();  
    FontInfo          fontInfo[3];
    int               myDrawChar(int x, int y, unsigned char c,  int color, int bg,FontInfo &info);
    
    FontInfo          *currentFont;
  // extended API
public:  
    void  setFontFamily(const GFXfont *small, const GFXfont *medium, const GFXfont *big);
    void  myDrawString(const char *st, int padd_up_to_n_pixels=0);
    void  setFontSize(FontSize size);
    //
    void drawBitmap(int width, int height, int wx, int wy, int fgcolor, int bgcolor, const uint8_t *data);
    void drawRLEBitmap(int widthInPixel, int height, int wx, int wy, int fgcolor, int bgcolor, const uint8_t *data);   // width is in bytes  
    virtual void pushColors(uint16_t *data, int len, boolean first);
    virtual void push2Colors(uint8_t *data, int len, boolean first,uint16_t fg, uint16_t bg) {};
};

#endif
