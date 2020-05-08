/***************************************************
 STM32 duino based firmware for DSO SHELL/150
 *  * GPL v2
 * (c) mean 2019 fixounet@free.fr
 ****************************************************/

#include "dso_includes.h"
#include "dso_test_signal.h"

extern void splash(void);
#define ADC_CR1_FASTINT 0x70000

//--
extern Adafruit_TFTLCD_8bit_STM32 *tft;
extern DSOControl *controlButtons;
extern int ints;
extern DSOADC    *adc;
extern testSignal *myTestSignal;
//
static float voltageScale;

static const TimeSettings tSettings[6]
{
    {false,"10us",     DSOADC::ADC_PRESCALER_2  ,ADC_SMPR_1_5,   4390},
    {false,"25us",     DSOADC::ADC_PRESCALER_2  ,ADC_SMPR_13_5,  5909},
    {false,"50us",     DSOADC::ADC_PRESCALER_2  ,ADC_SMPR_55_5,  4496},
    {false,"100us",    DSOADC::ADC_PRESCALER_4  ,ADC_SMPR_55_5,  4517},
    {false,"500us",    DSOADC::ADC_PRESCALER_4  ,ADC_SMPR_239_5, 6095},
    {false,"1ms",      DSOADC::ADC_PRESCALER_8  ,ADC_SMPR_239_5, 6095}
};



//


 

extern float test_samples[256];
static int currentVSettings=5;
static int currentTSettings=0;

static void updateTimeScale();

/**
 * 
 */

static inline int fromSample(float v)
{
    v*=vSettings[currentVSettings].displayGain;              //1 Vol / div       
    v+=120;
    if(v>239) v=239;
    if(v<0) v=0;
    v=239-v;
    return (int)v;
}

static uint8_t waveForm[240];

static int currentDiv=3;
static bool voltageMode=true;
static uint32_t acquisitionTime;
extern uint32_t convTime;
static int expand;
static bool first=true;
static void processSamples(int nbSamples, uint16_t *data);
uint32_t reg1[10],reg2[10];


static void ysetADCs ()
{
  //  const adc_dev *dev = PIN_MAP[analogInPin].adc_device;
  ADC1->regs->CR2 |= ADC_CR2_CONT; // | ADC_CR2_DMA; // Set continuous mode and DMA
  ADC1->regs->CR1 |= ADC_CR1_FASTINT; // Interleaved mode
  ADC2->regs->CR2 |= ADC_CR2_CONT; // ADC 2 continuos
  ADC1->regs->CR2 |= ADC_CR2_SWSTART;
  
}

void testDualADC(void)
{

    DSODisplay::init();
    int reCounter=0;
    currentVSettings=8;
    controlButtons->setInputGain( vSettings[currentVSettings].gain); // x1.4
    tft->setTextSize(2);
    myTestSignal->setFrequency(50000); // 100 hz /10 ms period
    updateTimeScale();
    adc->setupADCs ();
    float xmin,xmax,avg;
    expand=4096;
    while(1)
    {

        int markStart,markEnd;
        CaptureStats stats;
        int count;
        int i=0;
        // Ask samples , taking expand into account
        adc->prepareDualDMASampling (PA0,ADC_SMPR_1_5,DSOADC::ADC_PRESCALER_2);            
        ysetADCs();
        adc->startDualDMASampling (PA0,240);
        
        
        __IO uint32_t *xadc=(uint32_t *)ADC1->regs;
        __IO uint32_t *xadc2=(uint32_t *)ADC2->regs;
        
        for(int i=0;i<10;i++)
        {
            reg1[i]=xadc[i];
            reg2[i]=xadc2[i];
        }
        
        while(1)
        {
            FullSampleSet   s;
            while(1)
            {
                if(adc->getSamples(s))
                {
                    processSamples(s.set1.samples,s.set1.data);
                    for(int i=0;i<10;i++)
                    {
                        reg1[i]=xadc[i];
                        reg2[i]=xadc2[i];
                    }        
                    break;
                }
                
            }
        }
    }

} 
void processSamples(int nbSamples, uint16_t *data)
{
    volatile int sum=0;
    for(int i=0;i<nbSamples;i++)
        sum+=data[i];
}
void updateTimeScale()
{
    adc->setTimeScale(tSettings[currentTSettings].rate,tSettings[currentTSettings].prescaler);     
    expand=tSettings[currentTSettings].expand4096;
    tft->setCursor(240, 50);
    tft->print(tSettings[currentTSettings].name);
}

// EOF    
