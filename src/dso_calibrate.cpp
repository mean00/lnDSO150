/***************************************************
 STM32 duino based firmware for DSO SHELL/150
 *  * GPL v2
 * (c) mean 2019 fixounet@free.fr
 ****************************************************/

#include "dso_global.h"
#include "dso_adc.h"
#include "dso_eeprom.h"
#include "dso_adc_gain.h"
#include "dso_adc_gain_priv.h"
#include "dso_gfx.h"
extern DSOADC                     *adc;

#define SHORT_PRESS(x) (controlButtons->getButtonEvents(DSOControl::x)&EVENT_SHORT_PRESS)

/**
 * 
 */
static void waitOk()
{
    while(!SHORT_PRESS(DSO_BUTTON_OK)) 
    {
        xDelay(10);
    }
}

/**
  */
static void printCalibrationTemplate( const char *st1, const char *st2)
{
    DSO_GFX::newPage("CALIBRATION");    
    DSO_GFX::center(st1,80);
    DSO_GFX::center(st2,120);
    DSO_GFX::bottomLine("and press @OK@");    
}

/**
 * 
 * @param cpl
 */
static void printCoupling(DSOControl::DSOCoupling cpl)
{static const char *coupling[3]={"currently : GND","currently : DC ","currently : AC "};    
    DSO_GFX::center(coupling[cpl],130);
      
}

static void  waitForCoupling(DSOControl::DSOCoupling target)
{
    DSOControl::DSOCoupling   cpl=(DSOControl::DSOCoupling)-1;    
    const char *st="Set input to DC";
    if(target==DSOControl::DSO_COUPLING_AC) st="Set input to AC";
    DSO_GFX::center(st,100);
    while(1)
    {
            controlButtons->updateCouplingState();
            DSOControl::DSOCoupling   newcpl=controlButtons->getCouplingState(); 
            if(newcpl==target) 
                tft->setTextColor(GREEN,BLACK);
            else  
                tft->setTextColor(RED,BLACK);

            if(cpl!=newcpl)
            {
                printCoupling(newcpl);
                cpl=newcpl;
            }
            if(cpl==target && SHORT_PRESS(DSO_BUTTON_OK))
            {               
                return;
            }
    }
}

void header(int color,const char *txt,DSOControl::DSOCoupling target)
{
    printCalibrationTemplate(txt,"");
    waitForCoupling(target);
    DSO_GFX::center("@- processing -@",120+60);
    return;
}
#define NB_SAMPLES 64
/**
 * 
 * @return 
 */
static int averageADCRead()
{
    // Start Capture
    adc->setupTimerSampling(); // switch to time
    adc->prepareTimerSampling(1000,1,ADC_SMPR_239_5, DSOADC::ADC_PRESCALER_8); // 1Khz
    adc->startTimerSampling(200);
    FullSampleSet fset;
    while(!adc->getSamples(fset))
    {
        
    };
    int nb=fset.set1.samples;
    int sum=0;
    for(int i=0;i<nb;i++)
    {
        sum+=fset.set1.data[i];
    }
    sum=(sum+(nb/2)-1)/nb;
    return sum;
}

/**
 * 
 * @param array
 * @param color
 * @param txt
 * @param target
 */
void doCalibrate(uint16_t *array,int color, const char *txt,DSOControl::DSOCoupling target)
{
    header(color,txt,target);     
    for(int range=0;range<DSO_NB_GAIN_RANGES;range++)
    {
        DSOInputGain::setGainRange((DSOInputGain::InputGainRange) range);
        xDelay(10);
        array[range]=averageADCRead();
    }
}

/**
 * 
 * @return 
 */
bool DSOCalibrate::zeroCalibrate()
{    
    tft->setFontSize(Adafruit_TFTLCD_8bit_STM32::MediumFont);  
    tft->setTextColor(WHITE,BLACK);
          
    
    adc->setTimeScale(ADC_SMPR_1_5,DSOADC::ADC_PRESCALER_2); // 10 us *1024 => 10 ms scan
    printCalibrationTemplate("Connect the 2 crocs","together");
    waitOk();    
    doCalibrate(calibrationDC,YELLOW,"",DSOControl::DSO_COUPLING_DC);       
    doCalibrate(calibrationAC,GREEN, "",DSOControl::DSO_COUPLING_AC);    
    DSOEeprom::write();         
    tft->fillScreen(0);    
    DSO_GFX::printxy(20,100,"Restart the unit.");
    while(1) {};
    return true;        
}
/**
 * 
 * @return 
 */
bool DSOCalibrate::decalibrate()
{    
    DSOEeprom::wipe();
    return true;        
}
/**
 * 
 * @return 
 */
typedef struct MyCalibrationVoltage
{
    const char *title;
    float   expected;
    DSOInputGain::InputGainRange range;    
};
/**
 */
MyCalibrationVoltage myCalibrationVoltage[]=
{    
    {"24V",     24,     DSOInputGain::MAX_VOLTAGE_8V},    // 2v/div range
    {"16V",     16,     DSOInputGain::MAX_VOLTAGE_4V},    // 2v/div range
    {"8V",      8,      DSOInputGain::MAX_VOLTAGE_2V},     // 1v/div range
    //{"3.2V",    3.2,    DSOInputGain::MAX_VOLTAGE_800MV},     // 500mv/div range => Saturates
    //{"1.6V",    1.6,    DSOInputGain::MAX_VOLTAGE_400MV},     // 200mv/div range    => Saturates
    {"1V",      1,      DSOInputGain::MAX_VOLTAGE_250MV},     // 1v/div range
    {"800mV",   0.8,    DSOInputGain::MAX_VOLTAGE_200MV},     // 100mv/div range
    {"320mV",   0.32,   DSOInputGain::MAX_VOLTAGE_80MV},     //  50mv/div range
    {"150mV",   0.15,   DSOInputGain::MAX_VOLTAGE_40MV},    //   20mv/div range
    {"80mV",    0.08,   DSOInputGain::MAX_VOLTAGE_20MV},     //  10mv/div range    
};



float performVoltageCalibration(const char *title, float expected,float defalt,float previous,int offset);
/**
 * 
 * @return 
 */
bool DSOCalibrate::voltageCalibrate()
{
    float fvcc=DSOADC::getVCCmv();
    tft->setFontSize(Adafruit_TFTLCD_8bit_STM32::MediumFont);  
    
    
    DSO_GFX::newPage("VOLTAGE CALIBRATION");
    DSO_GFX::bottomLine("press @OK@ when ready");
    
    waitForCoupling(DSOControl::DSO_COUPLING_DC);    
    
    // here we go
    
    adc_set_sample_rate(ADC2, ADC_SMPR_239_5);
    int nb=sizeof(myCalibrationVoltage)/sizeof(MyCalibrationVoltage);
    for(int i=0;i<nb;i++)
    {                
        DSOInputGain::InputGainRange  range=myCalibrationVoltage[i].range;
        DSOInputGain::setGainRange(range);
        float expected=myCalibrationVoltage[i].expected;        
        int dex=(int)range;
        float previous=(voltageFineTune[dex]*fvcc)/4096000.;
        float f=performVoltageCalibration(myCalibrationVoltage[i].title,
                                          expected,
                                          DSOInputGain::getMultiplier(),
                                          previous,
                                          DSOInputGain::getOffset(0));
        if(f)
            voltageFineTune[dex]=(f*4096000.)/fvcc;
        else
            voltageFineTune[dex]=0;
    }    
    // If we have both 100mv and 2v
    DSOEeprom::write();         
    tft->fillScreen(0);
    return true;         
}

/**
 * 
 * @param expected
 */
static void fineHeader(const char *title)
{          
    
    DSO_GFX::newPage("VOLTAGE CALIBRATION");        
    DSO_GFX::bottomLine(" @VOLT@:Default,@TRIG@:Keep,@OK@:Set");    
    
    DSO_GFX::center("Set input to ",36);
    tft->setTextColor(GREEN,BLACK);
    tft->myDrawString(title);
    tft->setTextColor(WHITE,BLACK);
     
    
}


void printInt(int x , int y, int value)
{
    char str[20];
    sprintf(str,"%d   ",value);
    tft->setCursor(x, y);
    tft->myDrawString(str);
}
/**
 * 
 * @param title
 * @param expected
 * @return 
 */
float performVoltageCalibration(const char *title, float expected,float defalt,float previous,int offset)
{
#define SCALEUP 1000000    
    fineHeader(title);
    
    DSO_GFX::printxy(200,70,"@  ADC  @");
    DSO_GFX::printxy(10,70, "@ Scale @");
    DSO_GFX::printxy(10,110,"@Default@");
    DSO_GFX::printxy(10,140,"@PrevVal@s");
    while(1)
    {   // Raw read
        int sum=averageADCRead();        
        sum-=offset;               
        float f=expected;
        if(!sum) f=0;
        else
                 f=expected/sum;        
        printInt      (200,90,sum);
        printInt      (10,90,f*SCALEUP);
        printInt      (10,130,defalt*SCALEUP);
        printInt      (10,160,previous*SCALEUP);
        
         
        if( SHORT_PRESS(DSO_BUTTON_OK))
             return f;
        if( SHORT_PRESS(DSO_BUTTON_VOLTAGE))
             return 0.;
        if( SHORT_PRESS(DSO_BUTTON_TRIGGER))
             return previous;
    }    
}
