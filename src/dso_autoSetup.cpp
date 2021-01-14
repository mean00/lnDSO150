/***************************************************
 STM32 duino based firmware for DSO SHELL/150
 * Try to auto set voltage range & frequency
 * Could be better, but that will do for now
 *  * GPL v2
 * (c) mean 2019 fixounet@free.fr
 ****************************************************/
#include "dso_includes.h"
#include "stopWatch.h"
extern float test_samples[256];

static bool autoSetupVoltage(bool setTrigger);
static bool autoSetupFrequency();

/**
 * 
 */
void        autoSetup()
{
    DSOCapture::stopCapture();
    DSODisplay::drawAutoSetup();
    // switch to free running mode
    
    DSOCapture::setTriggerMode(DSOCapture::Trigger_Run);
       
    DSOCapture::DSO_TIME_BASE timeBase=DSOCapture::DSO_TIME_BASE_1MS;
    DSOCapture::setTimeBase(timeBase);
    // voltage range
    
    if(!autoSetupVoltage(false))         
        goto end; // failed 
    DSODisplay::drawAutoSetupStep(1);    
    if(!autoSetupFrequency()) 
        goto end;
    DSODisplay::drawAutoSetupStep(2);    
    // redo voltage in case it was wrong the 1st time due to too high/too low fq
    if(!autoSetupVoltage(true)) 
        goto end; // failed
    DSODisplay::drawAutoSetupStep(3);    
end:    
    DSOCapture::stopCapture();
    return;
   
}

/**
 * 
 * @return 
 */
bool autoSetupVoltage(bool setTrigger)
{
    int voltage=DSOCapture::DSO_VOLTAGE_5V;
    DSOCapture::setVoltageRange((DSOCapture::DSO_VOLTAGE_RANGE)voltage);
    
    CaptureStats stats;
    StopWatch clock;
    clock.ok();
    int tries=NB_CAPTURE_VOLTAGE+2;
    while(!clock.elapsed(2000))
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
        
        if(tries--<0) return true; // did not converge ?
        // Are we over the max ?
        if(xmax>DSOCapture::getMaxVoltageValue() && voltage<DSOCapture::DSO_VOLTAGE_MAX) // saturation            
        {
            voltage=voltage+1; // yes, use higher scale
            DSOCapture::setVoltageRange((DSOCapture::DSO_VOLTAGE_RANGE)voltage);
            continue;
        }
        // is it too small ?, if so take a lower (more zoom) range
        if(xmax<DSOCapture::getMinVoltageValue() && voltage>1) // too small, voltage =0 means ground, we dont want it
        {
            voltage=voltage-1;
            DSOCapture::setVoltageRange((DSOCapture::DSO_VOLTAGE_RANGE)voltage);
            continue;
        }    
        // Set the trigger
        float med=(stats.xmin+stats.xmax)/2;
        if(setTrigger)
        {
            DSOCapture::setTriggerValue(med);
          //  DSOCapture::setTriggerMode(DSOCapture::Trigger_Rising);
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
    int timeBase=(int)DSOCapture::DSO_TIME_MIN;
    DSOCapture::setTimeBase((DSOCapture::DSO_TIME_BASE)timeBase);
    
    CaptureStats stats;
    StopWatch clock;
    clock.ok();
    int tries=20;
    while(1)
    {
        if(clock.elapsed(2000) || timeBase>DSOCapture::DSO_TIME_BASE_MAX)
        {
            break;
        }
    
        int n=DSOCapture::capture(240,test_samples,stats);
        if(!n)
            continue;
        if(tries--<0) return true; // did not converge ?
        if(stats.frequency>30)
        {
            // Try to get fq = 4 square
            int fq=DSOCapture::timeBaseToFrequency((DSOCapture::DSO_TIME_BASE)timeBase);
            if(0 || stats.frequency<= fq*4)
            {
                //
                // Readjust trigger
                float trigger=(stats.xmax+stats.xmin)/2.;
                DSOCapture::setTriggerValue(trigger);
                return true;
            }
        }
        timeBase++;
        if(timeBase>DSOCapture::DSO_TIME_BASE_MAX) break;
        DSOCapture::setTimeBase((DSOCapture::DSO_TIME_BASE)timeBase);
    } 
    DSOCapture::setTimeBase(DSOCapture::DSO_TIME_BASE_1MS);
    return false;
}
