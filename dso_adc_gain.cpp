
#include "dso_global.h"
#include "dso_adc_gain.h"
#include "dso_control.h"
#include "dso_adc.h"
#include "dso_adc_gain_priv.h"
// Compute default gain/attenuration depending on the scale
// G1a/B is driven by SENSEL3

const float G1a=10./11.;
const float G1b=10./1100.;
const float G2=2.;
const float G4=1.+1000./130.; // 130 for new board, 150 for old boards, R14 and R15

// G3 is driven by SENSEL0/1/2
const float G3[6]={1.,2.,4.,10.,20.,40.};



/**
 */
static DSOInputGain::InputGainRange currentRange;

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
uint16_t calibrationDC[DSO_NB_GAIN_RANGES+1];
uint16_t calibrationAC[DSO_NB_GAIN_RANGES+1];
float    voltageFineTune[DSO_NB_GAIN_RANGES+1];
float    multipliers[DSO_NB_GAIN_RANGES+1];

/**
 */
bool DSOInputGain::setGainRange(DSOInputGain::InputGainRange range)
{
    currentRange=range;
    controlButtons->setInputGain(gainMapping[(int)range]);
    return true;
}
/**
 * 
 * @return 
 */
DSOInputGain::InputGainRange DSOInputGain::getGainRange()
{
    return currentRange;
}
/**
 */
int                          DSOInputGain::getOffset(int dc0ac1)
{
    if(dc0ac1==0)
        return calibrationDC[(int)currentRange];
    else
        return calibrationAC[(int)currentRange];
}
/**
 * 
 * @return 
 */
float                        DSOInputGain::getMultiplier()
{
    return multipliers[(int)currentRange];
}

/**
 * 
 * @param mul
 * @param offset
 * @param sta
 */
static void computeMultiplier(float *mul,int offset,float sta)
{
    float v;
    for(int i=0;i<6;i++)
    {      
        if(!voltageFineTune[offset+i])
        {
            v=G3[i]/sta;
        }
        else
        {
            v=voltageFineTune[offset+i];
        }
        mul[i+offset]=v;
    }
}
/**
 * 
 * @return 
 */
bool DSOInputGain::readCalibrationValue()
{
    float fvcc=DSOADC::readVCCmv();            

    float stat;
    multipliers[0]=0;
    computeMultiplier(multipliers,1,G1a*G2*G4);
    computeMultiplier(multipliers,1+6,G1b*G2*G4);
    

    float mu=fvcc/4096000.;
    for(int i=0;i<DSO_NB_GAIN_RANGES;i++)
        multipliers[i]=multipliers[i]*mu;
    return true;
}
