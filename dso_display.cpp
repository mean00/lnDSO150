/***************************************************
 STM32 duino based firmware for DSO SHELL/150
 *  * GPL v2
 * (c) mean 2019 fixounet@free.fr
 ****************************************************/
#include "dso_global.h"
#include "dso_display.h"
#include "pattern.h"

static DSODisplay::MODE_TYPE mode=DSODisplay::VOLTAGE_MODE;
/**
 */
uint8_t prevPos[256];
uint8_t prevSize[256];
static char textBuffer[24];



/**
 */
static const char *fq2Text(int fq)
{
    static char buff[16];
    float f=fq;
    const char *suff="";
#define STEP(x,t)  if(f>x)     {suff=t;f/=x;}else

    STEP(1000000,"M")
    STEP(1000,"K")
    {}
    
    sprintf(buff,"%3.1f%sH",f,suff);
    return buff;
}
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
    if(last>DSO_WAVEFORM_HEIGHT) last=DSO_WAVEFORM_HEIGHT;
    int next;
    int start,sz;
    
    for(int j=1;j<count;j++)
    {
        int next=data[j]; // in pixel
        sz=abs(next-last);        
        start=min(last,next);
                
        if(!sz)
        {
            if(next>=(DSO_WAVEFORM_HEIGHT)) next=DSO_WAVEFORM_HEIGHT;
            sz=1;
        }
        if(!start)
        {
            start=1;
        }


        uint16_t *bg=(uint16_t *)defaultPattern;
        if(!(j%24)) bg=(uint16_t *)darkGreenPattern;

        // cleanup prev draw
        tft->setAddrWindow(j,prevPos[j]+DSO_WAVEFORM_OFFSET,j,DSO_WAVEFORM_HEIGHT+DSO_WAVEFORM_OFFSET);
        tft->pushColors(((uint16_t *)bg)+prevPos[j],   prevSize[j],true);
        tft->drawFastVLine(j,start+DSO_WAVEFORM_OFFSET,sz,YELLOW);
        prevSize[j]=sz;
        prevPos[j]=start;
        last=next;
    }
} 
//-
#define SCALE_STEP 24
#define C_X 10
#define C_Y 8
#define CENTER_CROSS 1
/**
 * 
 */
void DSODisplay::drawGrid(void)
{
    uint16_t fgColor=(0xF)<<5;
    uint16_t hiLight=(0x1F)<<5;
    for(int i=0;i<=C_Y;i++)
    {
        tft->drawFastHLine(0,SCALE_STEP*i+DSO_WAVEFORM_OFFSET,SCALE_STEP*(C_X),fgColor);
    }
    for(int i=0;i<=C_X;i++)
    {

        tft->drawFastVLine(SCALE_STEP*i,DSO_WAVEFORM_OFFSET,SCALE_STEP*C_Y,fgColor);
    }

    tft->drawFastHLine(0,DSO_WAVEFORM_OFFSET,SCALE_STEP*C_X,hiLight);    
    tft->drawFastHLine(0,DSO_WAVEFORM_OFFSET+DSO_WAVEFORM_HEIGHT+2,SCALE_STEP*C_X,hiLight);
    tft->drawFastHLine(0,DSO_WAVEFORM_OFFSET+DSO_WAVEFORM_HEIGHT/2,SCALE_STEP*C_X,hiLight);
    
    
    tft->drawFastVLine(0,DSO_WAVEFORM_OFFSET,SCALE_STEP*C_Y,hiLight);
    tft->drawFastVLine(C_X*SCALE_STEP,DSO_WAVEFORM_OFFSET,SCALE_STEP*C_Y,hiLight);   
    tft->drawFastVLine((C_X*SCALE_STEP)/2,DSO_WAVEFORM_OFFSET,SCALE_STEP*C_Y,hiLight);
    
    
//    tft->drawFastHLine(SCALE_STEP*(C/2-CENTER_CROSS),SCALE_STEP*5,SCALE_STEP*CENTER_CROSS*2,WHITE);
//    tft->drawFastVLine(SCALE_STEP*5,SCALE_STEP*(C/2-CENTER_CROSS),SCALE_STEP*CENTER_CROSS*2,WHITE);

        

}
/**
 * 
 * @param drawOrErase
 */    
void  DSODisplay::drawVerticalTrigger(bool drawOrErase,int column)
{
    if(drawOrErase)
     tft->drawFastVLine(column,1,DSO_WAVEFORM_HEIGHT-1,RED);
    else
    {
        uint16_t *bg=(uint16_t *)defaultPattern;
        if(!(column%24)) 
            bg=(uint16_t *)darkGreenPattern;
        tft->setAddrWindow(column,0,column,DSO_WAVEFORM_HEIGHT);
        tft->pushColors(((uint16_t *)bg),   DSO_WAVEFORM_HEIGHT,true);
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
    if(line>DSO_WAVEFORM_HEIGHT-1) line=DSO_WAVEFORM_HEIGHT-1;
    if(drawOrErase)
        tft->drawFastHLine(1,1+line,DSO_WAVEFORM_WIDTH-1,BLUE);
    else
    {
        uint16_t *bg=(uint16_t *)defaultPattern;
        if(!(line%24)) 
            bg=(uint16_t *)darkGreenPattern;
        tft->setAddrWindow(0,1+line,DSO_WAVEFORM_WIDTH-1,1+line);
        tft->pushColors(((uint16_t *)bg),   DSO_WAVEFORM_WIDTH,true);
    }
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
    
#define AND_ONE_A(x,y) { tft->setCursor(DSO_INFO_START_COLUMN+2, DSO_HEIGHT_OFFSET+y*DSO_CHAR_HEIGHT); tft->myDrawString(x,DSO_INFO_MAX_WIDTH);}        
#define AND_ONE_T(x,y) { tft->setCursor(DSO_INFO_START_COLUMN+4, DSO_HEIGHT_OFFSET+y*DSO_CHAR_HEIGHT); tft->myDrawString(x,DSO_INFO_MAX_WIDTH);}    
#define AND_ONE_F(x,y) { sprintf(textBuffer,"%02.2f",x);tft->setCursor(DSO_INFO_START_COLUMN+4, DSO_HEIGHT_OFFSET+y*DSO_CHAR_HEIGHT); tft->myDrawString(textBuffer,DSO_INFO_MAX_WIDTH);}    
    
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
    

#define BG_COLOR GREEN    
        tft->drawFastVLine(DSO_INFO_START_COLUMN, 0,DSO_WAVEFORM_HEIGHT-1,BG_COLOR);
        tft->drawFastVLine(319, 0,DSO_WAVEFORM_HEIGHT-1,BG_COLOR);
        

    tft->setTextColor(BLACK,BG_COLOR);
    AND_ONE_A("Mini",0);   
    AND_ONE_A("Maxi",2);   
    AND_ONE_A("Avrg",4);
    AND_ONE_A("Freq",6);
    AND_ONE_A("Trigg",8);
    AND_ONE_A("Offst",10);
    tft->setTextColor(BG_COLOR,BLACK);
}
#define LOWER_BAR_PRINT(x,y) { tft->setCursor(y*64, 240-18); tft->myDrawString(x,DSO_INFO_MAX_WIDTH);}            

/**
 * 
 * @param volt
 */
void  DSODisplay::printOffset(float volt)
{
    AND_ONE_F(volt,11);      
}

/**
 * 
 * @param mode
 * @param volt
 */
void DSODisplay::printTriggerValue( float volt)
{    
    
    AND_ONE_F(volt,9);      
}

/**
 * 
 */
void DSODisplay::printVoltTimeTriggerMode(const char *volt, const char *time,DSOCapture::TriggerMode tmode)
{
    const char *st="????";
    switch(tmode)
    {
        case DSOCapture::Trigger_Rising: st="UP";break;
        case DSOCapture::Trigger_Falling: st="DOWN";break;
        case DSOCapture::Trigger_Both: st="BOTH";break;
        default:
            xAssert(0);
            break;
    }
    const char *coupling="??";
     switch(controlButtons->getCouplingState())
    {
        case DSOControl::DSO_COUPLING_GND:
            coupling="GND";
            break;
        case DSOControl::DSO_COUPLING_DC:
            coupling="DC";
            break;
        case DSOControl::DSO_COUPLING_AC:
            coupling="AC";
            break;
    }    
    
#define SELECT(md)   { if(md==mode) tft->setTextColor(BLACK,BG_COLOR); else  tft->setTextColor(BG_COLOR,BLACK);}
#define LOWER_BAR(mode,st,column) {SELECT(mode);    LOWER_BAR_PRINT(st,column);}
    
    LOWER_BAR(VOLTAGE_MODE,volt,0);
    LOWER_BAR(TIME_MODE,time,1);
    LOWER_BAR(TRIGGER_MODE,st,2);
    tft->setTextColor(BG_COLOR,BLACK);
    LOWER_BAR_PRINT(coupling,3);
    
    if(mode&0x80)
    {
        tft->setTextColor(BLACK,BLUE);
        switch(mode & 0x7f)
        {
            case TRIGGER_MODE:    AND_ONE_A("Trigg",8);break;
            case VOLTAGE_MODE:    AND_ONE_A("Offst",10);break;
        }
        tft->setTextColor(BG_COLOR,BLACK);
    }
        
}
/**
 * 
 * @return 
 */
DSODisplay::MODE_TYPE DSODisplay::getMode() 
{
    return mode;
}
/**
 * 
 * @param t
 */
void  DSODisplay::setMode(MODE_TYPE t) 
{
    if((mode&0x80) ^ !(t&0x80)) // leaving or entering alt
    {
        drawStatsBackGround();
    }
    mode=t;
}



// EOF