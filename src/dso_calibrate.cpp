/***************************************************
 STM32 duino based firmware for DSO SHELL/150
 *  * GPL v2
 * (c) mean 2021 fixounet@free.fr
 ****************************************************/

#include "lnArduino.h"
#include "dso_control.h"
#include "dso_calibrate.h"
#include "dso_adc_gain.h"
#include "dso_gfx.h"
#include "lnADC.h"
#include "dso_capture_input.h"
#include "gd32/nvm_gd32.h"
//
/**
 * 
 * @return 
 */
struct MyCalibrationVoltage
{
    const char *title;
    const float   expected;
    const DSOInputGain::InputGainRange range;    
};
/**
 */
static const MyCalibrationVoltage myCalibrationVoltage[]=
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

static void printCalibrationTemplate( const char *st1, const char *st2);
static void doCalibrate(uint16_t *array,int color, const char *txt,DSOControl::DSOCoupling target);

//
extern lnTimingAdc              *_adc;
extern DSOControl               *control;

#define SHORT_PRESS(x) (control->getButtonEvents(DSOControl::x)&EVENT_SHORT_PRESS)

/**
 * 
 */
static void waitOk()
{
    while(!SHORT_PRESS(DSO_BUTTON_OK)) 
    {
        control->getRotaryValue(); // purge
        xDelay(10);
    }
}


/**
 * 
 * @return 
 */
bool DSOCalibrate::zeroCalibrate()
{
    // Catch control callback
    DSOControl::ControlEventCb *oldCb=control->getCb();
    bool r=zeroCalibrate_();
    control->changeCb(oldCb);
    return r;    
}

/**
 * 
 * @return 
 */
bool DSOCalibrate::zeroCalibrate_()
{        
    DSO_GFX::setBigFont(false);
    DSO_GFX::setTextColor(WHITE,BLACK);
              
    printCalibrationTemplate("Connect the 2 crocs","together");
    waitOk();    
    doCalibrate(calibrationDC,YELLOW,"",DSOControl::DSO_COUPLING_DC);       
    doCalibrate(calibrationAC,GREEN, "",DSOControl::DSO_COUPLING_AC);    
          
    DSO_GFX::clear(0);    
    DSO_GFX::printxy(20,100,"Restart the unit.");
    while(1) {};
    return true;        
}
/**
  */
void printCalibrationTemplate( const char *st1, const char *st2)
{
    DSO_GFX::newPage("CALIBRATION");    
    DSO_GFX::center(st1,4);
    DSO_GFX::center(st2,5);
    DSO_GFX::bottomLine("and press @OK@");    
}

/**
 * 
 * @param cpl
 */
static void printCoupling(DSOControl::DSOCoupling cpl)
{
    static const char *coupling[3]={"currently : GND","currently : DC ","currently : AC "};    
    DSO_GFX::center(coupling[cpl],5);      
}
/**
 * 
 * @param target
 */
static void  waitForCoupling(DSOControl::DSOCoupling target)
{
    DSOControl::DSOCoupling   cpl=(DSOControl::DSOCoupling)-1;    
    const char *st="Set input to DC";
    if(target==DSOControl::DSO_COUPLING_AC) st="Set input to AC";
    DSO_GFX::center(st,4);
    while(1)
    {
            DSOControl::DSOCoupling   newcpl=control->getCouplingState(); 
            if(newcpl==target) 
                DSO_GFX::setTextColor(GREEN,BLACK);
            else  
                DSO_GFX::setTextColor(RED,BLACK);

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
/**
 * 
 * @param color
 * @param txt
 * @param target
 */
void header(int color,const char *txt,DSOControl::DSOCoupling target)
{
    printCalibrationTemplate(txt,"");
    waitForCoupling(target);
    DSO_GFX::center("@- processing -@",6);
    return;
}
#define NB_SAMPLES 64
/**
 * 
 * @return 
 */
static int averageADCRead()
{
#define NB_POINTS 16
    uint16_t samples[NB_POINTS];
   _adc->multiRead(16,samples);
   int sum=0;
   for(int i=0;i<NB_POINTS;i++)
   {
       sum+=samples[i];
   }
   sum=(sum+(NB_POINTS/2-1))/NB_POINTS;
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
        control->setInputGain(range);
#warning FIXME        
        //DSOInputGain::setGainRange((DSOInputGain::InputGainRange) range);
        xDelay(10);
        array[range]=averageADCRead();
        Logger("Range : %d val=%d\n",range,array[range]);
    }
}

/**
 * 
 * @return 
 */
bool DSOCalibrate::decalibrate()
{    
    
    return true;        
}

#if 0
/**
 * 
 * @return 
 */
bool DSOCalibrate::voltageCalibrate()
{
    // Catch control callback
    DSOControl::ControlEventCb *oldCb=control->getCb();
    bool r=voltageCalibrate_();
    control->changeCb(oldCb);
    return r;
    
}
/**
 * 
 * @return 
 */
bool DSOCalibrate::voltageCalibrate_()
{    
    // reset completely the adc    
    float fvcc= _adc->getVcc();

    DSO_GFX::setBigFont(true);
    DSO_GFX::newPage("VOLTAGE CALIBRATION");
    DSO_GFX::bottomLine("press @OK@ when ready");
    
    waitForCoupling(DSOControl::DSO_COUPLING_DC);    
    
    // here we go
    
    int nb=sizeof(myCalibrationVoltage)/sizeof(MyCalibrationVoltage);
    for(int i=0;i<nb;i++)
    {                
        DSOInputGain::InputGainRange  range=myCalibrationVoltage[i].range;
        DSOInputGain::setGainRange(range);
       
    }    
    // If we have both 100mv and 2v
    DSOEeprom::write();         
    DSO_GFX::clear(0);
    return true;         
}
#endif
// EOF