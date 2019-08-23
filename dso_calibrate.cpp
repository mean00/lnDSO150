/***************************************************
 STM32 duino based firmware for DSO SHELL/150
 *  * GPL v2
 * (c) mean 2019 fixounet@free.fr
 ****************************************************/

#include "dso_global.h"
#include "dso_adc.h"
#include "dso_eeprom.h"
extern DSOADC                     *adc;
extern float                       voltageFineTune[16];
extern uint16_t directADC2Read(int pin);
/**
 * 
 */
static void waitOk()
{
    while(!(controlButtons->getButtonEvents(DSOControl::DSO_BUTTON_OK)&EVENT_SHORT_PRESS)) {}
}

/**
 * 
 * @param array
 */
static void printxy(int x, int y, const char *t)
{
    tft->setCursor(x, y);
    tft->myDrawString(t);
}
/**
  */
static void printCalibrationTemplate( const char *st1, const char *st2)
{
    tft->fillScreen(BLACK);  
    printxy(0,10,"======CALIBRATION========");
    printxy(40,100,st1);
    printxy(8,120,st2);
    printxy(80,200,"and press OK");
}

/**
 * 
 * @param cpl
 */
static void printCoupling(DSOControl::DSOCoupling cpl)
{
    static const char *coupling[3]={"current : GND","current : DC ","current : AC "};
    printxy(40,130,coupling[cpl]);
      
}

void header(int color,const char *txt,DSOControl::DSOCoupling target)
{
    printCalibrationTemplate(txt,"");
    DSOControl::DSOCoupling   cpl=(DSOControl::DSOCoupling)-1;
    printxy(220,40," switch /\\");
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
            if(cpl==target && controlButtons->getButtonEvents(DSOControl::DSO_BUTTON_OK)&EVENT_SHORT_PRESS)
            {
                tft->setTextColor(BLACK,GREEN);
                printxy(160-8*8,160,"- processing -");
                tft->setTextColor(WHITE,BLACK);
                return;
            }
    }
}
#define NB_SAMPLES 64
void doCalibrate(uint16_t *array,int color, const char *txt,DSOControl::DSOCoupling target)
{
    
    printCalibrationTemplate("Connect probe to ground","(connect the 2 crocs together)");
    header(color,txt,target); 
    
    for(int range=0;range<14;range++)
    {
        controlButtons->setInputGain(range);        
        xDelay(10);
        int sum=0;
        for(int i=0;i<NB_SAMPLES;i++)
        {
            sum+=directADC2Read(analogInPin);
            xDelay(2);
        }
        sum=(sum+(NB_SAMPLES/2)-1)/NB_SAMPLES;
        array[range]=sum;
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
    
    
    adc->setupADCs ();
    adc->setTimeScale(ADC_SMPR_1_5,ADC_PRE_PCLK2_DIV_2); // 10 us *1024 => 10 ms scan
    printCalibrationTemplate("Connect the 2 crocs together.","");
    waitOk();
    doCalibrate(calibrationDC,YELLOW,"Set switch to *GND*",DSOControl::DSO_COUPLING_GND);                     
    doCalibrate(calibrationDC,YELLOW,"Set switch to *DC*",DSOControl::DSO_COUPLING_DC);       
    doCalibrate(calibrationAC,GREEN, "Set switch to *AC*",DSOControl::DSO_COUPLING_AC);
    //while(1) {};
    DSOEeprom::write();         
    tft->fillScreen(0);
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
    DSOCapture::DSO_VOLTAGE_RANGE range;    
    int         tableOffset;            // offset in fineVoltageMultiplier table
    float       expectedVoltage;
};

MyCalibrationVoltage myCalibrationVoltage[]=
{
    {"10v", DSOCapture::DSO_VOLTAGE_2V,    9,  10.0}, // 2v/div range
    {"5.0v",DSOCapture::DSO_VOLTAGE_1V,    8,  5.0},     // 1v/div range
    {"2.5v",DSOCapture::DSO_VOLTAGE_500MV, 7,  2.5},     // 500mv/div range
    {"1.0v",DSOCapture::DSO_VOLTAGE_200MV, 6,  1.0},     // 200mv/div range
    {"0.5v",DSOCapture::DSO_VOLTAGE_100MV, 5,  0.5},     // 100mv/div range
    
};

/**
 * 
 * @param title
 * @param expected
 * @return 
 */
static float performVoltageCalibration(const char *title, float expected)
{
    return 0.0;
}
/**
 * 
 * @return 
 */
bool DSOCalibrate::voltageCalibrate()
{
    tft->setFontSize(Adafruit_TFTLCD_8bit_STM32::MediumFont);  
    tft->setTextColor(WHITE,BLACK);
    
    
    adc->setupADCs ();
    adc->setTimeScale(ADC_SMPR_1_5,ADC_PRE_PCLK2_DIV_2); // 10 us *1024 => 10 ms scan
    for(int i=0;i<sizeof(myCalibrationVoltage)/sizeof(MyCalibrationVoltage);i++)
    {
        capture->setVoltageRange(myCalibrationVoltage[i].range);
        float f=performVoltageCalibration(myCalibrationVoltage[i].title,myCalibrationVoltage[i].expectedVoltage);
        voltageFineTune[myCalibrationVoltage[i].tableOffset]=f;
    }    
    DSOEeprom::write();         
    tft->fillScreen(0);
    return true;         
}
