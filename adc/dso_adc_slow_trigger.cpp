
#include "dso_adc.h"
#include "dso_adc_priv.h"
#include "fancyLock.h"

/**
 * 
 * @param count
 * @param buffer
 * @return 
 */
bool DSOADC::startTriggeredTimerSampling (int count,uint32_t triggerADC)
{
   
    return false;
    
} 
void DSOADC::Timer_Trigger_Event() 
{    
    nbTimer++;
    instance->timerTriggerCapture();
}


/**
 * \fn timerCapture
 * \brief this is one is called by a timer interrupt
 */
void DSOADC::timerTriggerCapture()
{    

}

// EOF

