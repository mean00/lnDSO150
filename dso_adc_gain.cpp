
#include "dso_global.h"
#include "dso_adc_gain.h"
#include "dso_control.h"
/**
 */
static const int gainMapping[16]=
{
    1 , // GND          [0]
    
    8+4, // x14   1mv   [1]
    8+6, // x7           [2]
    8+7, // x3.5        [3]
    8+0, // x1.4        [4]
    8+5, // x0.7        [5]
    8+3, // x0.35       [6]
    
    4, // /7    100 mv  [7]
    6, // /14   200 mv  [8]
    7, // /29   500 mv  [9]
    0, // /71   1v      [10]
    5, // /143  2v      [11]
    3,  // /286 5v      [12]
    
    3,3,3 // Filler
};

/**
 */
bool DSOInputGain::setGainRange(DSOInputGain::InputGainRange range)
{
    controlButtons->setInputGain(gainMapping[(int)range]);
    return true;
}