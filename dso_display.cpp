/***************************************************
 STM32 duino based firmware for DSO SHELL/150
 *  * GPL v2
 * (c) mean 2019 fixounet@free.fr
 * 
 * /!\ All related to waveform (waveform + trigger) are automatically centered,
 * you dont need to apply offset
 * 
 * 
 ****************************************************/
#include "dso_global.h"
#include "dso_display.h"
#include "pattern.h"
#include "stopWatch.h"


#define AUTOCAL_BOX_WIDTH   200
#define AUTOCAL_BOX_HEIGHT  80
#define AUTOCAL_BORDER      10
#define AUTOCAL_COLOR       WHITE
#define LED_X               120
#define LED_Y               2
#define LED_R               10

static DSO_ArmingMode oldMode=DSO_CAPTURE_MODE_INVALIDE;

StopWatch triggerWatch;

static DSODisplay::MODE_TYPE mode=DSODisplay::VOLTAGE_MODE;
/**
 */
uint8_t prevPos[256];
uint8_t prevSize[256];
static char textBuffer[24];

//-
#define SCALE_STEP 24
#define C_X 10
#define C_Y 8
#define CENTER_CROSS 1

static const uint16_t *getBackGround(int line)
{
    const uint16_t *bg=(uint16_t *)defaultPattern;
    if(!(line%SCALE_STEP)) 
            bg=(uint16_t *)darkGreenPattern;
    return bg;
}


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
    triggerWatch.elapsed(0);
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
    
    if(count<3) return;
    for(int j=1;j<count-1;j++)
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

        const uint16_t *bg=getBackGround(j);
        // cleanup prev draw
        tft->setAddrWindow(j,prevPos[j]+DSO_WAVEFORM_OFFSET,j,DSO_WAVEFORM_HEIGHT+DSO_WAVEFORM_OFFSET);
        tft->pushColors(((uint16_t *)bg)+prevPos[j],   prevSize[j],true);
        tft->drawFastVLine(j,start+DSO_WAVEFORM_OFFSET,sz,YELLOW);
        prevSize[j]=sz;
        prevPos[j]=start;
        last=next;
    }    
} 
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
    
    
    tft->fillCircle(LED_X +LED_R, LED_Y+LED_R, LED_R, GREEN);


}
/**
 * 
 * @param drawOrErase
 */    
void  DSODisplay::drawVerticalTrigger(bool drawOrErase,int column)
{
    if(drawOrErase)
     tft->drawFastVLine(column,DSO_WAVEFORM_OFFSET+1,DSO_WAVEFORM_HEIGHT-1,RED);
    else
    {
        const uint16_t *bg=getBackGround(column);
        tft->setAddrWindow(column,1+DSO_WAVEFORM_OFFSET,column,DSO_WAVEFORM_HEIGHT+DSO_WAVEFORM_OFFSET-1);
        tft->pushColors(((uint16_t *)bg),   DSO_WAVEFORM_HEIGHT,true);
    }
}
/**
 * 
 * @param drawOrErase
 * @param line
 */
extern int debugUp, debugDown;
void  DSODisplay::drawVoltageTrigger(bool drawOrErase, int line)
{
    if(line<1) line=1;
    if(line>DSO_WAVEFORM_HEIGHT-1) line=DSO_WAVEFORM_HEIGHT-1;
    
    if(drawOrErase)
        tft->drawFastHLine(1,1+line,DSO_WAVEFORM_WIDTH-2,BLUE);
    else
    {
        const uint16_t *bg=getBackGround(line);
        tft->setAddrWindow( 0,                  1+line,
                            DSO_WAVEFORM_WIDTH, 1+line);
        tft->pushColors(((uint16_t *)bg),   DSO_WAVEFORM_WIDTH,true);
    }
#if 0    
     tft->setCursor(240,16);tft->print(debugUp);
     tft->setCursor(240,36);tft->print(debugDown);
#endif
      
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
    if(stats.frequency>0)
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
        tft->drawFastVLine(DSO_INFO_START_COLUMN, 0,240,BG_COLOR);
        tft->drawFastVLine(319, 0,240,BG_COLOR);
        

    tft->setTextColor(BLACK,BG_COLOR);
    AND_ONE_A("Mini",0);   
    AND_ONE_A("Maxi",2);   
    AND_ONE_A("Avrg",4);
    AND_ONE_A("Freq",6);
    AND_ONE_A("Trigg",8);
    AND_ONE_A("Offst",10);
    tft->setTextColor(BG_COLOR,BLACK);
    oldMode=DSO_CAPTURE_MODE_INVALIDE;
    
}
#define LOWER_BAR_PRINT(x,y) { tft->setCursor(y*64, 240-18); tft->myDrawString(x);}            
#define LOWER_BAR_PRINT_NCHARS(x,y,n) { tft->setCursor(y*64, 240-18); tft->myDrawString(x,n*18);}            

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
        case DSOCapture::Trigger_Rising: st="UP  ";break;
        case DSOCapture::Trigger_Falling: st="DOWN";break;
        case DSOCapture::Trigger_Both: st="BOTH";break;
        case DSOCapture::Trigger_Run: st="RUN ";break;
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
#define LOWER_BAR_NCHAR(mode,st,column) {SELECT(mode);    LOWER_BAR_PRINT_NCHARS(st,column,4);}    
    LOWER_BAR(VOLTAGE_MODE,volt,0);
    LOWER_BAR(TIME_MODE,time,1);
    LOWER_BAR_NCHAR(TRIGGER_MODE,st,2);
    tft->setTextColor(BG_COLOR,BLACK);
    LOWER_BAR_PRINT_NCHARS(coupling,3,3);
    
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

 void DSODisplay::drawAutoSetup(void )
 {
     tft->fillRoundRect(320/2-AUTOCAL_BOX_WIDTH/2,
                         240/2-AUTOCAL_BOX_HEIGHT/2, 
                         AUTOCAL_BOX_WIDTH,AUTOCAL_BOX_HEIGHT,
                         4,AUTOCAL_COLOR);      
 }
void DSODisplay::drawAutoSetupStep(int i )
 {
    int pg=((AUTOCAL_BOX_WIDTH-AUTOCAL_BORDER*2)*i)/3;
     tft-> fillRoundRect(320/2-AUTOCAL_BOX_WIDTH/2+AUTOCAL_BORDER,
                         240/2-AUTOCAL_BOX_HEIGHT/2+AUTOCAL_BORDER, 
                         pg,
                         AUTOCAL_BOX_HEIGHT-AUTOCAL_BORDER*2,
                         4,
                         0);    
     
 }

static void drawLed(int color)
{

#define BORDER_WIDTH 3
    tft->fillRect(  LED_X+BORDER_WIDTH+1,
                    LED_Y+BORDER_WIDTH+1,
                    LED_R*2-BORDER_WIDTH*2,
                    LED_R*2-BORDER_WIDTH*2,color);
}

void  DSODisplay::triggered(bool gotIt)
{
    if(gotIt)
        drawLed(GREEN);
    else      
        drawLed(BLACK);
}
/**
 * 
 * @param mode
 */

void  DSODisplay::drawArmingMode(DSO_ArmingMode mode,bool triggered)
{
    drawArmingMode(mode);
    drawTrigger(triggered);
}
/**
 * 
 * @param mode
 */
void  DSODisplay::drawArmingMode(DSO_ArmingMode mode)
{
    
    if(oldMode==mode)
        return;
    oldMode=mode;
#define ARM_WIDTH (108)
#define TRIGGER_STEP 128 // 50 ms step
     const char *smode="??";
     switch(mode)
     {
        case   DSO_CAPTURE_SINGLE_ARMED:    smode="SNGL:WAIT";DSODisplay::triggered(false);break;
        case   DSO_CAPTURE_SINGLE_CAPTURED: smode="SNGL:TRG"; DSODisplay::triggered(true);break;
        case   DSO_CAPTURE_MULTI :  
                                {
                                    smode="MULTI";      
                                    if(triggered)
                                        triggerWatch.ok();                                    
                                    int d=triggerWatch.msSinceOk()/TRIGGER_STEP;
                                    int color=0x3f>>d;
                                    drawLed(color<<5);
                                }
                                break;
        default:
                xAssert(0);
                break;
     }
     tft->setCursor(4,0);
     tft->myDrawString(smode,ARM_WIDTH);
}
/**
 * 
 * @param triggered
 */

void  DSODisplay::drawTrigger(bool triggered)
{
    if(oldMode!=DSO_CAPTURE_MULTI) return;
    if(triggered)
        triggerWatch.ok();                                    
    int d=triggerWatch.msSinceOk()/TRIGGER_STEP;
    int color=0x3f>>d;
    drawLed(color<<5);
}

// EOF