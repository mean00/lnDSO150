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
#include "lnArduino.h"
#include "lnCpuID.h"
#include "dso_display.h"
#include "pattern_decl.h"
#include "simpler9341.h"
#include "math.h"
#include "dso_colors.h"
#include "DSO_portBArbitrer.h"
#include "assets/gfx/generated/splash_decl.h"
#include "dso_version.h"

#define XMAX(x, y) (((x) > (y)) ? (x) : (y))
#define XMIN(x, y) (((x) < (y)) ? (x) : (y))

#define LIGHT_GREEN          ILI_MK_COLOR(0,0x2f,0) // careful the colors are also in genpattern.py
#define DARK_GREEN           ILI_MK_COLOR(0,0x1f,0)

#define AUTOCAL_BOX_WIDTH   200
#define AUTOCAL_BOX_HEIGHT  80
#define AUTOCAL_BORDER      10
#define AUTOCAL_COLOR       WHITE

#define SCALE_STEP 20
#define C_X 12
#define C_Y 10
#define CENTER_CROSS 1

#define LINE_OFFSET 4

#define DSO_LOW_BAR_BUTTON  68

#define DSO_CHAR_HEIGHT 20
#define DSO_HEIGHT_OFFSET 1
#define DSO_INFO_START_COLUMN (248)
#define DSO_INFO_MAX_WIDTH  (320-DSO_INFO_START_COLUMN-8)

static void prettyPrint(float x,int mx);



#define TITLE_SQUARE(x,color)         tft->square(color, \
            DSO_INFO_START_COLUMN,             DSO_HEIGHT_OFFSET+(HeaderLine[x]-1)*DSO_CHAR_HEIGHT, \
            320-DSO_INFO_START_COLUMN,         DSO_CHAR_HEIGHT);


static ili9341 *tft;
extern DSO_portArbitrer *arbitrer;
static DSO_ArmingMode oldMode=DSO_CAPTURE_MODE_INVALIDE;
static DSODisplay::MODE_TYPE mode=DSODisplay::VOLTAGE_MODE;
/**
 */
uint8_t prevPos[256];
uint8_t prevSize[256];
static char textBuffer[24];

extern const uint8_t *getSplash();
//


#define MIN_ROW                 0
#define MAX_ROW                 1
#define FREQ_ROW                2
#define TRIGGER_ROW             3
#define VOLTAGE_OFFSET_ROW      4
#define ARMING_ROW              5

#define RIGHT_HAND_OFFSET 1

static const int DisplayLine[10]=
{
RIGHT_HAND_OFFSET+2,//#define MIN_ROW                 0
RIGHT_HAND_OFFSET+3,//#define MAX_ROW                 2
RIGHT_HAND_OFFSET+5,//#define FREQ_ROW                4
RIGHT_HAND_OFFSET+7,//#define TRIGGER_ROW             6
RIGHT_HAND_OFFSET+9,//#define VOLTAGE_OFFSET_ROW      8
};
static const int HeaderLine[10]=
{
RIGHT_HAND_OFFSET+1,//#define MIN_ROW                 0
RIGHT_HAND_OFFSET+1, // MAX
RIGHT_HAND_OFFSET+4,//#define FREQ_ROW                4
RIGHT_HAND_OFFSET+6,//#define TRIGGER_ROW             6
RIGHT_HAND_OFFSET+8,//#define VOLTAGE_OFFSET_ROW      8
RIGHT_HAND_OFFSET+10, // ARMING
};

class AutoGfx
{
public:
    AutoGfx()
    {
        arbitrer->beginLCD();
    }
    ~AutoGfx()
    {
        arbitrer->endLCD();
    }
};
//-

static void VALUE_Y_POSITION(int x)
{
      tft->setCursor(DSO_INFO_START_COLUMN+2, DSO_HEIGHT_OFFSET+(DisplayLine[x])*DSO_CHAR_HEIGHT-5);
}

static const uint16_t *getBackGround(int line,bool &repeat)
{
    const uint16_t *bg=(uint16_t *)defaultPattern;
    repeat=false;
    if(!(line%SCALE_STEP)) 
    {
            bg=(uint16_t *)darkGreenPattern;
            repeat=true;
    }
    if(line==DSO_WAVEFORM_WIDTH/2)
    {
            bg=(uint16_t *)lightGreenPattern;
            repeat=true;
    }
    return bg;
}
/**
 * 
 * @param line
 * @return 
 */
static const uint16_t *getHzBackGround(int line,bool &repeat)
{
    const uint16_t *bg=(uint16_t *)horizontal;
    repeat=false;
    if(!(line%SCALE_STEP)) 
    {
            bg=(uint16_t *)darkGreenPattern;
            repeat=true;
    }
    if(line==DSO_WAVEFORM_WIDTH/2)
    {
            bg=(uint16_t *)lightGreenPattern;
            repeat=true;
    }
    return bg;
}


/**
 * 
 * @param line
 * @param info
 */


static void printMeasurement(int line, const float f)
{    
    VALUE_Y_POSITION(line);    
    prettyPrint(f,320-4-DSO_INFO_START_COLUMN);
}
/**
 */
static const char *fq2Text(int fq)
{
    static char buff[16];
    float  f=fq;
    const char *suff="";
#define STEP(x,t)  if(fq>((x*11)/10))     {suff=t;f/=(float)x;}else

    STEP(1000000,"M")
    STEP(1000,"K")
    {        
    }            
            
            
    if(*suff)
        sprintf(buff,"%2.1f%s",f,suff);
    else
        sprintf(buff,"%d%s",(int)f,suff);
    return buff;
}
/**
 * 
 */
void DSODisplay::init(ili9341 *d)
{
    tft=d;
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
    AutoGfx autogfx;
    
    //tft->fillScreen(0);
    int last=data[0];
    last=XMIN(last,DSO_WAVEFORM_HEIGHT-1);
    int next;
    int start,sz;
    
    if(count<3) return;
    for(int j=1;j<count-1;j++)
    {
        
        int color=YELLOW;
        int next=data[j]; // in pixel
        if(next==0)
        {
            next=1;
            color=RED;
        }
        if(next>DSO_WAVEFORM_HEIGHT-1)
        {
            next=DSO_WAVEFORM_HEIGHT-1;
            color=RED;
        }
        start=XMIN(last,next);                
        sz=next-last;
        if(sz<0) sz=-sz;
        if(!sz)
        {            
            sz=1;
        }
        bool repeat=false;
        const uint16_t *bg=getBackGround(j,repeat);
        // cleanup prev draw
        tft->setAddress(    j,
                            prevPos[j]+DSO_WAVEFORM_OFFSET,
                            1,
                            prevSize[j]); 
        if(repeat)       
            tft->floodWords(prevSize[j],bg[0]);
        else
            tft->pushColors(prevSize[j],((uint16_t *)bg)+prevPos[j]);        
        // Now draw the real one
        
        tft->VLine(j,start+DSO_WAVEFORM_OFFSET,sz,color);
        prevSize[j]=sz;
        prevPos[j]=start;
        last=next;
    }    
} 
/**
 * 
 * @param mn
 * @param mx
 */
void  DSODisplay::drawMinMax(float mn, float mx)
{
    AutoGfx autogfx;
    tft->setTextColor(ILI_WHITE,ILI_BLACK);
    printMeasurement(MIN_ROW, mn);
    printMeasurement(MAX_ROW, mx);    
}
/**
 * 
 */
void  DSODisplay::drawFq(int f)
{
    AutoGfx autogfx;
    VALUE_Y_POSITION(FREQ_ROW);     
    if(f==0)
    {
        tft->square(0, 
            DSO_INFO_START_COLUMN,             DSO_HEIGHT_OFFSET+(DisplayLine[FREQ_ROW]-1)*DSO_CHAR_HEIGHT, 
            320-DSO_INFO_START_COLUMN,         DSO_CHAR_HEIGHT);           
        tft->print("-");
        return;            
    }
    const char *t= fq2Text(f)  ;
    tft->setTextColor(WHITE,BLACK);        
    tft->printUpTo(t,320-DSO_INFO_START_COLUMN);
}

/**
 * 
 */
void DSODisplay::cleanup()
{
     AutoGfx autogfx;
     tft->setTextColor(LIGHT_GREEN,0);
     tft->fillScreen(0);
}

/**
 * 
 */
void DSODisplay::drawGrid(void)
{
    AutoGfx autogfx;
    uint16_t fgColor=DARK_GREEN;
    uint16_t hiLight=LIGHT_GREEN;
    for(int i=0;i<=C_Y;i++)
    {
        tft->HLine(0,
                SCALE_STEP*i+DSO_WAVEFORM_OFFSET,
                SCALE_STEP*(C_X),
                fgColor);
    }
    for(int i=0;i<=C_X;i++)
    {
        tft->VLine(SCALE_STEP*i,DSO_WAVEFORM_OFFSET,SCALE_STEP*C_Y,fgColor);
    }
    // Draw internal + cross in bright green
    tft->HLine(0,DSO_WAVEFORM_OFFSET,SCALE_STEP*C_X,hiLight);    
    tft->HLine(0,DSO_WAVEFORM_OFFSET+DSO_WAVEFORM_HEIGHT-1,SCALE_STEP*C_X,hiLight);
    tft->HLine(0,DSO_WAVEFORM_OFFSET+DSO_WAVEFORM_HEIGHT/2,SCALE_STEP*C_X,hiLight);
    
    
    tft->VLine(0,DSO_WAVEFORM_OFFSET,SCALE_STEP*C_Y,hiLight);
    tft->VLine(C_X*SCALE_STEP,DSO_WAVEFORM_OFFSET,SCALE_STEP*C_Y,hiLight);   
    tft->VLine((C_X*SCALE_STEP)/2,DSO_WAVEFORM_OFFSET,SCALE_STEP*C_Y,hiLight);
}
/**
 * 
 * @param drawOrErase
 */    
void  DSODisplay::drawVerticalTrigger(bool drawOrErase,int column)
{
    if(drawOrErase)
     tft->VLine(column,DSO_WAVEFORM_OFFSET+1,DSO_WAVEFORM_HEIGHT-1,RED);
    else
    {
        bool repeat;
        const uint16_t *bg=getHzBackGround(column,repeat);
        tft->setAddress(column,1+DSO_WAVEFORM_OFFSET,
                        1,DSO_WAVEFORM_HEIGHT-1);
        if(repeat)
            tft->floodWords(DSO_WAVEFORM_HEIGHT,bg[0]);
        else
            tft->pushColors(DSO_WAVEFORM_HEIGHT,((uint16_t *)bg));
    }
}
/**
 * 
 * @param drawOrErase
 * @param line
 */

void  DSODisplay::drawVoltageTrigger(bool drawOrErase, int line)
{
    AutoGfx autogfx;
    
    line=DSO_WAVEFORM_HEIGHT/2+DSO_WAVEFORM_OFFSET-line;
    if(line<1) line=1;
    if(line>DSO_WAVEFORM_HEIGHT-1) line=DSO_WAVEFORM_HEIGHT-1;
    
    if(drawOrErase)
    {
        tft->HLine(1,1+line,DSO_WAVEFORM_WIDTH-2,BLUE);
    }
    else // erase
    {
        bool repeat;
        const uint16_t *bg=getHzBackGround(line,repeat);
        tft->setAddress( 0,                  1+line,
                            DSO_WAVEFORM_WIDTH, 1);
        if(repeat)
            tft->floodWords(DSO_WAVEFORM_WIDTH,bg[0]);
        else
            tft->pushColors(DSO_WAVEFORM_WIDTH,((uint16_t *)bg));
    }
}

/**
 * 
 * @param stats
 */

void prettyPrint(float x,int maxW=0)
{
    float a=fabs(x);
  
    if(a<0.8)
    {
        if(a==0.0)
        {
            sprintf(textBuffer,"0");
        }else
        if(a<0.001)
            sprintf(textBuffer,"%03du",(int)(1000000.*x));
        else
            sprintf(textBuffer,"%03dm",(int)(1000.*x));
    }
    else
        sprintf(textBuffer,"%02.2f",x);
    if(maxW)
        tft->printUpTo(textBuffer,maxW);
    else
        tft->print(textBuffer);
}


/**
 * 
 */
static __attribute__ ((noinline)) void drawInfo(int line, const char *info,int color)
{
    tft->print(DSO_INFO_START_COLUMN+2, DSO_HEIGHT_OFFSET+(line+1)*DSO_CHAR_HEIGHT-LINE_OFFSET,info);
}
/**
 * 
 */
static __attribute__ ((noinline))  void drawInfoHeader(int line, const char *info,int color)
{
    TITLE_SQUARE(line,color);
    tft->print(DSO_INFO_START_COLUMN+2, DSO_HEIGHT_OFFSET+HeaderLine[line]*DSO_CHAR_HEIGHT-LINE_OFFSET,info);
}


/**
 * 
 * @param mode
 * @param volt
 */
void DSODisplay::printTriggerValue( float volt,bool hilight)
{    
    AutoGfx autogfx;    
    if(hilight)
        tft->setTextColor(BLACK,WHITE);
    else
        tft->setTextColor(WHITE,BLACK);
    printMeasurement(TRIGGER_ROW, volt);
}


/**
 * 
 * @param mode
 * @param volt
 */
void DSODisplay::printOffsetValue( float volt,bool hilight)
{    
    AutoGfx autogfx;    
    if(hilight)
        tft->setTextColor(BLACK,WHITE);
    else
        tft->setTextColor(WHITE,BLACK);
    printMeasurement(VOLTAGE_OFFSET_ROW, volt);
}

/**
 * 
 */
 struct HeaderDesc
    {
        int16_t     row;
        const char *data;
    };

  static const HeaderDesc Headers[]=
    {
        {MIN_ROW,           "MinMx"},
        {FREQ_ROW,          "Freq"},
        {TRIGGER_ROW,       "Trigg"},
        {VOLTAGE_OFFSET_ROW,"Offset"},
        {-1,""},
    };

void DSODisplay::drawStatsBackGround()
{
    AutoGfx autogfx;
#define BG_STATS_HEIGHT (240-20)
#define BG_COLOR LIGHT_GREEN    
    tft->VLine(DSO_INFO_START_COLUMN, 0,BG_STATS_HEIGHT,BG_COLOR);
    tft->VLine(319, 0,BG_STATS_HEIGHT,BG_COLOR);
    tft->setTextColor(BLACK,BG_COLOR);  
    const HeaderDesc *h=Headers;
    while(h->row!=-1)
    {
        drawInfoHeader(h->row,     h->data,BG_COLOR);
        h++;
    }
    tft->setTextColor(BG_COLOR,BLACK);
    oldMode=DSO_CAPTURE_MODE_INVALIDE;    
}


void lowBarPrint(int column, const char *st)
{
    AutoGfx autogfx;
    
    tft->setCursor((column-1)*DSO_LOW_BAR_BUTTON+1, 240-2); 
    tft->square(0,(column-1)*DSO_LOW_BAR_BUTTON+1, 240-20,DSO_LOW_BAR_BUTTON,20);
    tft->print(st);
}
    

static void genericDraw(int column,const char *v,bool highlight)
{
    if(highlight)
    {
       tft->setTextColor(BLACK, LIGHT_GREEN );
    }else
    {
        tft->setTextColor(LIGHT_GREEN,BLACK);
    }    
    lowBarPrint(column,v);

    int fg,bg,button;

    if(highlight)
    {
        fg=ILI_BLACK;
        button= ILI_BLUE ;
        bg=ILI_BLACK;
    }else
    {
        fg=LIGHT_GREEN;
        button=ILI_BLACK;
        bg=ILI_BLACK;
    }
    AutoGfx autogfx;
    //(int x, int y, int w, int h, const char *text, int fontColor,int buttonColor,int bgColor)
    tft->centeredButton( (column-1)*DSO_LOW_BAR_BUTTON+1, 240 - 20, 
                        DSO_LOW_BAR_BUTTON,20, v,fg,button,bg);
    tft->setTextColor(LIGHT_GREEN,BLACK);                        
}
/**
 * 
 * @param v
 * @param highlight
 */
void DSODisplay::drawVolt(const char *v, bool highlight)    { genericDraw(VOLTAGE_MODE,v,highlight);}
void DSODisplay::drawTrigger(const char *v, bool highlight) { genericDraw(TRIGGER_MODE,v,highlight);}
void DSODisplay::drawTime(const char *v, bool highlight)    { genericDraw(TIME_MODE,   v,highlight);}
void DSODisplay::drawCoupling(const char *v, bool highlight){ genericDraw(STAT_MODE, v,highlight);}
void DSODisplay::drawArming(const char *v, bool highlight)  { genericDraw(ARMING_MODE ,   v,highlight);}

#if 0
/**
 * 
 * @param mode
 */
void  DSODisplay::drawArmingTriggeredMode(DSO_ArmingMode arming,bool triggered)
{       
    const char *armingString="?";
    switch(arming)
    {
        case DSO_CAPTURE_SINGLE:    armingString="Single";  break;
        case DSO_CAPTURE_MULTI:     armingString="Repeat";  break;
        case DSO_CAPTURE_CONTINUOUS:armingString="Run";     break;
            default:                xAssert(0);             break;
    }   
    int color;
    AutoGfx autogfx;    
    if(triggered)
        color=WHITE;
    else
        color=ILI_RED;
    tft->setTextColor(color,BLACK);
    VALUE_Y_POSITION(ARMING_ROW);    
    tft->printUpTo(armingString,320-DSO_INFO_START_COLUMN);
}
#endif
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
     AutoGfx autogfx;
     tft->fillRoundRect(320/2-AUTOCAL_BOX_WIDTH/2,
                         240/2-AUTOCAL_BOX_HEIGHT/2, 
                         AUTOCAL_BOX_WIDTH,AUTOCAL_BOX_HEIGHT,
                         4,AUTOCAL_COLOR,0);      
 }
void DSODisplay::drawAutoSetupStep(int i )
 {
    AutoGfx autogfx;
    int pg=((AUTOCAL_BOX_WIDTH-AUTOCAL_BORDER*2)*i)/3;
     tft-> fillRoundRect(320/2-AUTOCAL_BOX_WIDTH/2+AUTOCAL_BORDER,
                         240/2-AUTOCAL_BOX_HEIGHT/2+AUTOCAL_BORDER, 
                         pg,
                         AUTOCAL_BOX_HEIGHT-AUTOCAL_BORDER*2,
                         4,
                         AUTOCAL_COLOR,AUTOCAL_COLOR);    
     
 }

/**
 * 
 */
void  DSODisplay::printString(int x, int y, const char *text)
{
    AutoGfx autogfx;
    
    tft->setFontSize(ili9341::MediumFont);
    tft->setTextColor(WHITE,BLACK);
    tft->setCursor(x, y);
    tft->print(text);              
    tft->setFontSize(ili9341::SmallFont);
}
/**
 * 
 */
void  DSODisplay::clearScreen()
{
 AutoGfx autogfx;
    
    tft->fillScreen(BLACK);   
}
/**
 * 
 */

void DSODisplay::drawSplash()
{
    AutoGfx autogfx;
    
    tft->fillScreen(BLACK);   
    tft->drawHSBitmap(splash_width,splash_height,2,2, WHITE,BLACK,getSplash());
    tft->setFontSize(ili9341::SmallFont);
    tft->setTextColor(WHITE,BLACK);
    tft->setCursor(140, 64);
    tft->print("lnDSO150");              
    tft->setCursor(140, 84);
#ifdef USE_RXTX_PIN_FOR_ROTARY        
        tft->print("USB  Version");              
#else
        tft->print("RXTX Version");              
#endif
    char bf[20];
    sprintf(bf,"%d.%02d",DSO_VERSION_MAJOR,DSO_VERSION_MINOR);
    tft->setCursor(140, 64+20*2);        
    tft->print(bf);       
    tft->setCursor(140, 64+20*4);
    tft->print(lnCpuID::idAsString());         
    tft->setCursor(140, 64+20*5);
    sprintf(bf,"%d Mhz",lnCpuID::clockSpeed()/1000000);
    tft->print(bf);         
    xDelay(100);
}

// EOF
