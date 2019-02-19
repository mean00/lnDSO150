#include "dso_global.h"
#include "dsoDisplay.h"
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
    
