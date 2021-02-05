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
    float  f=fq;
    const char *suff="";
#define STEP(x,t)  if(f>x)     {suff=t;f/=(float)x;}else

    STEP(1000000,"M")
    STEP(1000,"K")
    {}
    sprintf(buff,"%2.1f%s",f,suff);
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
        
    //tft->fillCircle(LED_X +LED_R, LED_Y+LED_R, LED_R, GREEN);
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

static void prettyPrint(float x,int mx)
{
    float a=fabs(x);
  
    if(a<0.8)
    {
        if(a<0.001)
            sprintf(textBuffer,"%03du",(int)(1000000.*x));
        else
            sprintf(textBuffer,"%03dm",(int)(1000.*x));
    }
    else
        sprintf(textBuffer,"%02.2f",x);
  tft->myDrawString(textBuffer,mx);   
}

void DSODisplay::drawStats(CaptureStats &stats)
{
    
#define AND_ONE_A(x,y) { tft->setCursor(DSO_INFO_START_COLUMN+2, DSO_HEIGHT_OFFSET+y*DSO_CHAR_HEIGHT); tft->myDrawString(x,DSO_INFO_MAX_WIDTH);}        
#define AND_ONE_T(x,y) { tft->setCursor(DSO_INFO_START_COLUMN+4, DSO_HEIGHT_OFFSET+y*DSO_CHAR_HEIGHT); tft->myDrawString(x,DSO_INFO_MAX_WIDTH);}    
#define AND_ONE_F(x,y) { tft->setCursor(DSO_INFO_START_COLUMN+4, DSO_HEIGHT_OFFSET+y*DSO_CHAR_HEIGHT);prettyPrint(x,DSO_INFO_MAX_WIDTH);}    
    if(stats.saturation)
    {
           tft->setTextColor(RED,BLACK);
    }else
    {
           tft->setTextColor(GREEN,BLACK);
    }
    AND_ONE_F(stats.xmin,3);
    AND_ONE_F(stats.xmax,5);
    AND_ONE_F(stats.avg,1);      
    tft->setTextColor(GREEN,BLACK);
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
    AND_ONE_A("Min",2);   
    AND_ONE_A("Max",4);   
    AND_ONE_A("Avrg",0);
    AND_ONE_A("Freq(H)",6);
    AND_ONE_A("Trigg",8);
    AND_ONE_A("Offst",10);
    tft->setTextColor(BG_COLOR,BLACK);
    oldMode=DSO_CAPTURE_MODE_INVALIDE;
    
}

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

#define LOWER_BAR_PRINT(x,y) { tft->setCursor(y*64, 240-18); tft->myDrawString(x,64);}            
#define LOWER_BAR_PRINT_NCHARS(x,y,n) { tft->setCursor(y*64, 240-18); tft->myDrawString(x,n*18);}            
    
#define HIGHER_BAR_PRINT(x,y) { tft->setCursor(y*64, 1); tft->myDrawString(x,64);}            

#define SELECT(md)   { if(md==mode) tft->setTextColor(BLACK,BG_COLOR); else  tft->setTextColor(BG_COLOR,BLACK);}
#define LOWER_BAR(mode,st,column) {SELECT(mode);    LOWER_BAR_PRINT(st,column);}
#define LOWER_BAR_NCHAR(mode,st,column) {SELECT(mode);    LOWER_BAR_PRINT_NCHARS(st,column,4);}    
   
#define HIGHER_BAR(mode,st,column) {SELECT(mode);HIGHER_BAR_PRINT(st,column);}

/**
 * 
 */
void DSODisplay::printVoltTimeTriggerMode(const char *volt, const char *time,DSOCapture::TriggerMode tmode,DSO_ArmingMode arming)
{
    const char *st="????";
    switch(tmode)
    {
        case DSOCapture::Trigger_Rising:    st="UP  ";break;
        case DSOCapture::Trigger_Falling:   st="DOWN";break;
        case DSOCapture::Trigger_Both:      st="BOTH";break;
        case DSOCapture::Trigger_Run:       st="NONE ";break;
        default:            xAssert(0);            break;
    }
    const char *coupling="??";
     switch(controlButtons->getCouplingState())
    {
        case DSOControl::DSO_COUPLING_GND:  coupling="GND";break;
        case DSOControl::DSO_COUPLING_DC:   coupling=" DC"; break;
        case DSOControl::DSO_COUPLING_AC:   coupling=" AC"; break;
        default:            xAssert(0);            break;
    }    
    LOWER_BAR(VOLTAGE_MODE,volt,0);
    LOWER_BAR(TIME_MODE,time,1);
    LOWER_BAR_NCHAR(TRIGGER_MODE,st,2);
    
    SELECT(ARMING_MODE);
    drawArmingMode(arming);
   
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

/**
 * 
 * @param mode
 */
void  DSODisplay::drawArmingMode(DSO_ArmingMode arming)
{       
    const char *armingString="?";
    switch(arming)
    {
        case DSO_CAPTURE_SINGLE: armingString="SING";break;
        case DSO_CAPTURE_MULTI: armingString="REPT";break;
        case DSO_CAPTURE_CONTINUOUS: armingString="CONT";break;
            default:            xAssert(0);            break;
    }    
    HIGHER_BAR_PRINT(armingString,0);
}
/**
 * 
 * @param triggered
 */
DSO_ArmingMode lastMode=DSO_CAPTURE_MODE_INVALIDE;
bool lastTriggered=false;

void  DSODisplay::drawTriggeredState(DSO_ArmingMode mode, bool triggered)
{
    if(lastMode==mode && lastTriggered==triggered) return; // nothing to do
    lastMode=mode;
    lastTriggered=triggered;
    
    if(mode!=DSO_CAPTURE_SINGLE)
    {
       tft->setTextColor(BLACK,BLACK);  
       tft->setCursor(90, 0); 
       tft->myDrawString("Triggd",90);
       tft->setTextColor(BG_COLOR,BLACK);  
       return;
    }
    const char *s="Wait...";
    
    int bg=BLACK;
    int fg=BG_COLOR;;
        
    if(triggered)
    {
        s="Triggd";
        tft->setTextColor(bg,fg);
    }else
    {
         tft->setTextColor(fg,bg);
    }
    tft->setCursor(90, 0); 
    tft->myDrawString(s,90);
    tft->setTextColor(fg,bg);
}


// EOF