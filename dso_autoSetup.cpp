/***************************************************
 STM32 duino based firmware for DSO SHELL/150
 *  * GPL v2
 * (c) mean 2019 fixounet@free.fr
 ****************************************************/
#include "dso_includes.h"
#include "stopWatch.h"
extern float test_samples[256];

static bool autoSetupVoltage();
static bool autoSetupFrequency();

/**
 * 
 */
void        autoSetup()
{
    // swith to free running mode
    DSOCapture::stopCapture();
    DSOCapture::setTriggerMode(DSOCapture::Trigger_Run);
       
    DSOCapture::DSO_TIME_BASE timeBase=DSOCapture::DSO_TIME_BASE_5MS;
    DSOCapture::setTimeBase(timeBase);
    // voltage range
    
    if(!autoSetupVoltage()) return; // failed
    autoSetupFrequency();
    // frequency
   
}

/**
 * 
 * @return 
 */
bool autoSetupVoltage()
{
    int voltage=DSOCapture::DSO_VOLTAGE_5V;
    DSOCapture::setVoltageRange((DSOCapture::DSO_VOLTAGE_RANGE)voltage);
    
    CaptureStats stats;
    StopWatch clock;
    clock.ok();
    while(!clock.elapsed(1000))
    {
        int n=DSOCapture::capture(240,test_samples,stats);
        if(!n)
            continue;
        
        float  xmin= stats.xmin;
        float  xmax= stats.xmax;
        
        xmin=fabs(xmin);
        xmax=fabs(xmax);
        if(xmin>xmax) 
            xmax=xmin;
        clock.ok();
        
        // Are we over the max ?
        if(xmax>DSOCapture::getMaxVoltageValue() && voltage<DSOCapture::DSO_VOLTAGE_MAX) // saturation            
        {
            voltage=voltage+1; // yes, use higher scale
            DSOCapture::setVoltageRange((DSOCapture::DSO_VOLTAGE_RANGE)voltage);
            continue;
        }
        if(xmax<DSOCapture::getMinVoltageValue() && voltage>0) // too small
        {
            voltage=voltage-1;
            DSOCapture::setVoltageRange((DSOCapture::DSO_VOLTAGE_RANGE)voltage);
            continue;
        }    
        return true;
    } 
    return false;
}


/**
 * 
 * @return 
 */
bool autoSetupFrequency()
{
    int timeBase=(int)DSOCapture::DSO_TIME_BASE_10US;
    DSOCapture::setTimeBase((DSOCapture::DSO_TIME_BASE)timeBase);
    
    CaptureStats stats;
    StopWatch clock;
    clock.ok();
    while(!clock.elapsed(1000) && timeBase<=DSOCapture::DSO_TIME_BASE_MAX)
    {
        int n=DSOCapture::capture(240,test_samples,stats);
        if(!n)
            continue;
        if(stats.frequency>50)
        {
            // Readjust trigger
            float trigger=(stats.xmax+stats.xmin)/2.;
            DSOCapture::setTriggerValue(trigger);
            return true;
        }
        timeBase++;
        DSOCapture::setTimeBase((DSOCapture::DSO_TIME_BASE)timeBase);
    } 
     DSOCapture::setTimeBase(DSOCapture::DSO_TIME_BASE_1MS);
    return false;
}
