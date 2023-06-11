/**
 Quick explanation how the gain works on the DSO 150

 * G1 is 10/11 (PA4=1) or 10/1100 (PA4=0)  aka SENSEL3
 * G2 is 2
 * G3 is /1 ... / 40 Depending on PA1/PA2/PA3 aka  SENSEL0/1/2
 * G4 = 1+1000/150 (1000/130 for newer board) =7.6

 * PA1/2/3  G3
 * 1   GND
 * 4   /1
 * 6   /2
 * 7   /4
 * 0   /10
 * 5   /20
 * 3  /40
 */

#include "lnADC.h"
#include "lnArduino.h"

#include "dso_adc_gain.h"
#include "dso_adc_gain_priv.h"
#include "dso_control.h"

extern DSOControl *control;

// Compute default gain/attenuation depending on the scale
// G1a/B is driven by SENSEL3

const float G1a = 10. / 11.;
const float G1b = 10. / 1100.;
const float G2 = 2.;
const float G4 = 1. + 1000. / 130.; // 130 for new board, 150 for old boards, R14 and R15

// G3 is driven by SENSEL0/1/2
const float G3[6] = {1., 2., 4., 10., 20., 40.};

/**
 */
static DSOInputGain::InputGainRange currentRange;

static const int gainMapping[DSO_NB_GAIN_RANGES] = {
    1, // GND             [0]

    8 + 4, // x14   20mv     [1]    G=13.9/1=13.9
    8 + 6, // x7    40       [2]    G=13.9/2=6.96
    8 + 7, // x3.5  80       [3]    G=13.9/4=3.5
    8 + 0, // x1.4  200      [4]    G=13.9/10=1.4
    8 + 5, // x0.7  250/400  [5]    G=13.9/20=0.7
    8 + 3, // x0.35 800      [6]    G=13.9/40=0.35

    4, // /7    2          [7]    G=0.138/1=*0.138
    6, // /14   4          [8]    G=0.138/1=*0.069
    7, // /29   8 v        [9]
    0, // /71   20         [10]
    5, // /143  40         [11]
    // 3,  // /286 80        [11]

    // 3,3,3 // Filler
};
uint16_t calibrationDC[DSO_NB_GAIN_RANGES + 1] = {0};
uint16_t calibrationAC[DSO_NB_GAIN_RANGES + 1] = {0};
float voltageFineTune[DSO_NB_GAIN_RANGES + 1];
float multipliers[DSO_NB_GAIN_RANGES + 1];
float raw_multipliers[DSO_NB_GAIN_RANGES + 1];

/**
 */
bool DSOInputGain::setGainRange(DSOInputGain::InputGainRange range)
{
    Logger("Setting new GainRange=%d\n", range);
    currentRange = range;
    control->setInputGain(gainMapping[(int)range]);
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
int DSOInputGain::getOffset(int dc0ac1)
{
    if (dc0ac1 == 0)
        return calibrationDC[(int)currentRange];
    else
        return calibrationAC[(int)currentRange];
}

uint16_t *DSOInputGain::getCalibrationTable(int dc0ac1)
{
    if (dc0ac1 == 0)
        return calibrationDC;
    else
        return calibrationAC;
}

/**
 *
 * @return
 */
float DSOInputGain::getMultiplier()
{
    return multipliers[(int)currentRange];
}

/**
 *
 * @param mul
 * @param offset
 * @param sta
 */
static void computeMultiplier(float *mul, int offset, float sta)
{
    float v;
    for (int i = 0; i < 6; i++)
    {

        v = raw_multipliers[i + offset] = G3[i] / sta;
        mul[i + offset] = v;
    }
}
/**
 *
 * @return
 */
bool DSOInputGain::readCalibrationValue()
{
    float fvcc = lnBaseAdc::getVcc();

    float stat;
    multipliers[0] = 0;
    computeMultiplier(multipliers, 1, G1a * G2 * G4);
    computeMultiplier(multipliers, 1 + 6, G1b * G2 * G4);

    float mu = fvcc / (4095. * 1000.);
    for (int i = 0; i < DSO_NB_GAIN_RANGES; i++)
        multipliers[i] = multipliers[i] * mu;
    return true;
}
