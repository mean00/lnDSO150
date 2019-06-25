
/*****************************************************************************/
// Fast block fill operation for fillScreen, fillRect, H/V line, etc.
// Requires setAddrWindow() has previously been called to set the fill
// bounds.  'len' is inclusive, MUST be >= 1.
/*****************************************************************************/
#define MAX_BLOCKS (39*8)
void Adafruit_TFTLCD_8bit_STM32::flood(uint16_t color, uint32_t len)
{
    
   int l=len;
   uint8_t  hi = color >> 8,  lo = color&0xff;
   if(hi==lo)
   {
    while(l>64) 
    {
        int blocks=l/64;       
         if(blocks>MAX_BLOCKS) 
         {
             blocks=MAX_BLOCKS;
         }
         fillSameBlock(hi,blocks);
         l-=blocks*64;
     }
     if(!l)
        return;     
     floodSmallSame(hi,l);           
     return;
   }
   
   while(l>64) 
   {
       int blocks=l/64;       
        if(blocks>MAX_BLOCKS) 
        {
            blocks=MAX_BLOCKS;
        }
        fillDifferentBlock(hi,lo,blocks);
        l-=blocks*64;
    }
    if(!l)
       return;
    floodSmall(color,l);         
}
/**
 * 
 */
void Adafruit_TFTLCD_8bit_STM32::fillSameBlock(uint8_t hi, int blocks)
{
  CS_ACTIVE_CD_COMMAND;
  floodPreamble();
  CD_DATA;
  write8(hi);
  write8(hi);
  while(blocks--) 
  {
      int i = 16; // 64 pixels/block / 4 pixels/pass
      do {
        WR_STROBE; WR_STROBE; WR_STROBE; WR_STROBE; // 2 bytes/pixel
        WR_STROBE; WR_STROBE; WR_STROBE; WR_STROBE; // x 4 pixels
      } while(--i);
    }
  CS_IDLE;
}
/**
 */
void Adafruit_TFTLCD_8bit_STM32::fillDifferentBlock(uint8_t hi, uint8_t lo, int blocks)
{
  CS_ACTIVE_CD_COMMAND;
  floodPreamble();
  // Write first pixel normally, decrement counter by 1
  CD_DATA;
  write8(hi);
  write8(lo);
  while(blocks--) 
  {
      int i = 16; // 64 pixels/block / 4 pixels/pass
      do {
        write8(hi); write8(lo); write8(hi); write8(lo);
        write8(hi); write8(lo); write8(hi); write8(lo);
      } while(--i);
  }    
  CS_IDLE;
}
/**
 */
void Adafruit_TFTLCD_8bit_STM32::floodSmall(uint16_t color, int len)
{
  uint16_t blocks;
  uint8_t  i, hi = color >> 8,
              lo = color;
    
  CS_ACTIVE_CD_COMMAND;
  floodPreamble();  
  CD_DATA;
  write8(hi);
  write8(lo);
  len--;

  i = len; // Less than 64 pix, no optimization
  while (i--) 
  { // write here the remaining data
    write8(hi); write8(lo);
  }
  
  CS_IDLE;
}
void Adafruit_TFTLCD_8bit_STM32::floodSmallSame(uint8_t color, int len)
{
  uint16_t blocks;
  uint8_t  i, hi = color >> 8,
              lo = color;
    
  CS_ACTIVE_CD_COMMAND;
  floodPreamble();  
  CD_DATA;
  write8(color);
  write8(color);
  len--;

  i = len; // Less than 64 pix, no optimization
  while (i--) 
  { // write here the remaining data
    WR_STROBE; WR_STROBE;
  }
  
  CS_IDLE;
}

/*****************************************************************************/
// Fast block fill operation for fillScreen, fillRect, H/V line, etc.
// Requires setAddrWindow() has previously been called to set the fill
// bounds.  'len' is inclusive, MUST be >= 1.
/*****************************************************************************/
void Adafruit_TFTLCD_8bit_STM32::flood2(uint16_t color, uint16_t bg,uint32_t len)
{
  uint16_t blocks;
  uint8_t  i, hi = color >> 8,
              lo = color;

  uint8_t hiBg=bg>>8;
  uint8_t loBg=bg&0xff;
  CS_ACTIVE_CD_COMMAND;
  floodPreamble();
  

  // Write first pixel normally, decrement counter by 1
  CD_DATA;
  write8(hi);
  write8(lo);
  len--;

  blocks = (uint16_t)(len / 64); // 64 pixels/block
   {
    while(blocks--) {
      i = 16; // 64 pixels/block / 4 pixels/pass
      do {
        write8(hi); write8(lo); 
        write8(hi); write8(lo); 
        write8(hi); write8(lo);
        write8(hiBg); write8(loBg);
      } while(--i);
    }
	i = len & 63;
    while (i--) { // write here the remaining data
      write8(hi); write8(lo);
    }
  }
  CS_IDLE;
}
