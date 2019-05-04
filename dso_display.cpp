/***************************************************
 STM32 duino based firmware for DSO SHELL/150
 *  * GPL v2
 * (c) mean 2019 fixounet@free.fr
 ****************************************************/
#include "dso_global.h"
#include "dso_display.h"
#include "pattern.h"
/**
 */
uint8_t prevPos[256];
uint8_t prevSize[256];
/**
 * 
 */
void DSODisplay::init()
{
    for(int i=0;i<256;i++)
    {
        prevPos[i]=120;
        prevSize[i]=1;
    }
}
/**
 * 
 * @param data
 */
void  DSODisplay::drawWaveForm(int count,const uint8_t *data)
{
    //tft->fillScreen(0);
    int last=data[0];
    if(!last) last=1;
    if(last>=238) last=238;
    int next;
    int start,sz;
    
    for(int j=1;j<count;j++)
    {
        int next=data[j]; // in pixel
        sz=abs(next-last);        
        start=min(last,next);
                
        if(!sz)
        {
            if(next>=237) next=237;
            sz=1;
        }
        if(!start)
        {
            start=1;
        }


        uint16_t *bg=(uint16_t *)defaultPattern;
        if(!(j%24)) bg=(uint16_t *)darkGreenPattern;

        // cleanup prev draw
        tft->setAddrWindow(j,prevPos[j],j,240);
        tft->pushColors(((uint16_t *)bg)+prevPos[j],   prevSize[j],true);
        tft->drawFastVLine(j,start,sz,YELLOW);
        prevSize[j]=sz;
        prevPos[j]=start;
        last=next;
    }
} 
//-
#define SCALE_STEP 24
#define C 10
#define CENTER_CROSS 1
/**
 * 
 */
void DSODisplay::drawGrid(void)
{
    uint16_t fgColor=(0xF)<<5;
    uint16_t hiLight=(0x1F)<<5;
    for(int i=0;i<=C;i++)
    {
        tft->drawFastHLine(0,SCALE_STEP*i,SCALE_STEP*C,fgColor);
        tft->drawFastVLine(SCALE_STEP*i,0,SCALE_STEP*C,fgColor);
    }
    tft->drawFastHLine(0,239,SCALE_STEP*C,hiLight);
    tft->drawFastHLine(0,120,SCALE_STEP*C,hiLight);
    tft->drawFastVLine(0,0,SCALE_STEP*C,hiLight);
    tft->drawFastVLine(240,0,SCALE_STEP*C,hiLight);
    tft->drawFastVLine(120,0,SCALE_STEP*C,hiLight);
    
    tft->drawFastHLine(SCALE_STEP*(C/2-CENTER_CROSS),SCALE_STEP*5,SCALE_STEP*CENTER_CROSS*2,WHITE);
    tft->drawFastVLine(SCALE_STEP*5,SCALE_STEP*(C/2-CENTER_CROSS),SCALE_STEP*CENTER_CROSS*2,WHITE);
}
/**
 * 
 * @param drawOrErase
 */    
void  DSODisplay::drawVerticalTrigger(bool drawOrErase,int column)
{
    if(drawOrErase)
     tft->drawFastVLine(column,1,239,RED);
    else
    {
        uint16_t *bg=(uint16_t *)defaultPattern;
        if(!(column%24)) 
            bg=(uint16_t *)darkGreenPattern;
        tft->setAddrWindow(column,0,column,240);
        tft->pushColors(((uint16_t *)bg),   240,true);
    }
}
/**
 * 
 * @param drawOrErase
 * @param line
 */
void  DSODisplay::drawVoltageTrigger(bool drawOrErase, int line)
{
    if(line<1) line=1;
    if(line>238) line=238;
    if(drawOrErase)
        tft->drawFastHLine(1,1+line,238,BLUE);
    else
    {
        uint16_t *bg=(uint16_t *)defaultPattern;
        if(!(line%24)) 
            bg=(uint16_t *)darkGreenPattern;
        tft->setAddrWindow(0,1+line,239,1+line);
        tft->pushColors(((uint16_t *)bg),   240,true);
    }
}




/**
 */
static const char *fq2Text(int fq)
{
    static char buff[16];
    float f=fq;
    const char *suff=" ";
#define STEP(x,t)  if(f>x)     {suff=t;f/=x;}else

    STEP(1000000,"M")
    STEP(1000,"K")
    {}
    
    sprintf(buff,"%03.0f%sHz",f,suff);
    return buff;
}
#define DSO_CHAR_HEIGHT 20
#define DSO_HEIGHT_OFFSET 1
#define DSO_INFO_START_COLUMN (248)
#define DSO_INFO_MAX_WIDTH  (320-DSO_INFO_START_COLUMN-8)


/**
 * 
 * @param stats
 */
void DSODisplay::drawStats(CaptureStats &stats)
{
    char bf[24];
#define AND_ONE_A(x,y) { tft->setCursor(DSO_INFO_START_COLUMN+2, DSO_HEIGHT_OFFSET+y*DSO_CHAR_HEIGHT); tft->myDrawString(x,DSO_INFO_MAX_WIDTH);}        
#define AND_ONE_T(x,y) { tft->setCursor(DSO_INFO_START_COLUMN+4, DSO_HEIGHT_OFFSET+y*DSO_CHAR_HEIGHT); tft->myDrawString(x,DSO_INFO_MAX_WIDTH);}    
#define AND_ONE_F(x,y) { sprintf(bf,"%02.2f",x);tft->setCursor(DSO_INFO_START_COLUMN+4, DSO_HEIGHT_OFFSET+y*DSO_CHAR_HEIGHT); tft->myDrawString(bf,DSO_INFO_MAX_WIDTH);}    
    
    AND_ONE_F(stats.xmin,1);
    AND_ONE_F(stats.xmax,3);
    AND_ONE_F(stats.avg,5);      
    if(stats.avg>0)
    {
        AND_ONE_T(fq2Text(stats.frequency),7);
    }else
    {
        AND_ONE_T("--",7);
    }
}
/**
 * 
 */
void DSODisplay::drawStatsBackGround()
{
    char bf[24];

#define BG_COLOR GREEN    
        tft->drawFastVLine(DSO_INFO_START_COLUMN, 0,240,BG_COLOR);
        tft->drawFastVLine(319, 0,240,BG_COLOR);
        

    tft->setTextColor(BLACK,BG_COLOR);
    AND_ONE_A("Min",0);   
    AND_ONE_A("Max",2);   
    AND_ONE_A("Avg",4);
    AND_ONE_A("Frq",6);
    AND_ONE_A("V/D",8);
    AND_ONE_A("T/D",10);
    tft->setTextColor(BG_COLOR,BLACK);
}
/**
 * 
 */
void DSODisplay::drawVoltTime(const char *volt, const char *time)
{
    tft->setTextColor(BG_COLOR,BLACK);
    AND_ONE_T(volt,9);
    AND_ONE_T(time,11);
}
// EOF