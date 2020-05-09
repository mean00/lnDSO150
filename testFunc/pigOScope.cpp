/***************************************************
 STM32 duino based firmware for DSO SHELL/150
 *  * GPL v2
 * (c) mean 2019 fixounet@free.fr
 ****************************************************/

#include "dso_includes.h"
#include "dso_test_signal.h"


static void setADCs ();

static void takeSamples ();
static void xDMA1_CH1_Event();
static void adc_dma_disable(const adc_dev * dev) ;
extern void splash(void);
static void adc_dma_enable(const adc_dev * dev) ;
static void zsetup();
static void zloop();
void snapshotRegisters();
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
    {DSOADC::ADC_CAPTURE_MODE_NORMAL,"10us",     DSOADC::ADC_PRESCALER_2  ,ADC_SMPR_1_5,   4390},
    {DSOADC::ADC_CAPTURE_MODE_NORMAL,"25us",     DSOADC::ADC_PRESCALER_2  ,ADC_SMPR_13_5,  5909},
    {DSOADC::ADC_CAPTURE_MODE_NORMAL,"50us",     DSOADC::ADC_PRESCALER_2  ,ADC_SMPR_55_5,  4496},
    {DSOADC::ADC_CAPTURE_MODE_NORMAL,"100us",    DSOADC::ADC_PRESCALER_4  ,ADC_SMPR_55_5,  4517},
    {DSOADC::ADC_CAPTURE_MODE_NORMAL,"500us",    DSOADC::ADC_PRESCALER_4  ,ADC_SMPR_239_5, 6095},
    {DSOADC::ADC_CAPTURE_MODE_NORMAL,"1ms",      DSOADC::ADC_PRESCALER_8  ,ADC_SMPR_239_5, 6095}
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

void testPigOsCope(void)
{

    DSODisplay::init();
    int reCounter=0;
    currentVSettings=8;
    controlButtons->setInputGain( vSettings[currentVSettings].gain); // x1.4
    tft->setTextSize(2);
    myTestSignal->setFrequency(50000); // 100 hz /10 ms period
    zsetup();
    while(1)
    {
        zloop();
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

// Analog input
#define ANALOG_MAX_VALUE 4096
const int8_t analogInPin = PA0;   // Analog input pin: any of LQFP44 pins (PORT_PIN), 10 (PA0), 11 (PA1), 12 (PA2), 13 (PA3), 14 (PA4), 15 (PA5), 16 (PA6), 17 (PA7), 18 (PB0), 19  (PB1)
float samplingTime = 0;
float displayTime = 0;


// Variables for the beam position
uint16_t signalX ;
uint16_t signalY ;
uint16_t signalY1;
int16_t xZoomFactor = 1;
// yZoomFactor (percentage)
int16_t yZoomFactor = 100; //Adjusted to get 3.3V wave to fit on screen
int16_t yPosition = 0 ;

// Startup with sweep hold off or on
boolean triggerHeld = 0 ;


unsigned long sweepDelayFactor = 1;
unsigned long timeBase = 200;  //Timebase in microseconds

// Screen dimensions
int16_t myWidth ;
int16_t myHeight ;

//Trigger stuff
boolean notTriggered ;

// Sensitivity is the necessary change in AD value which will cause the scope to trigger.
// If VAD=3.3 volts, then 1 unit of sensitivity is around 0.8mV but this assumes no external attenuator. Calibration is needed to match this with the magnitude of the input signal.

// Trigger is setup in one of 32 positions
#define TRIGGER_POSITION_STEP ANALOG_MAX_VALUE/32
// Trigger default position (half of full scale)
int32_t triggerValue = 2048; 

int16_t retriggerDelay = 0;
int8_t triggerType = 2; //0-both 1-negative 2-positive

//Array for trigger points
uint16_t triggerPoints[2];


// Serial output of samples - off by default. Toggled from UI/Serial commands.
boolean serialOutput = false;


// Samples - depends on available RAM 6K is about the limit on an STM32F103C8T6
// Bear in mind that the ILI9341 display is only able to display 240x320 pixels, at any time but we can output far more to the serial port, we effectively only show a window on our samples on the TFT.
# define maxSamples 512 //1024*6
uint32_t startSample = 0; //10
uint32_t endSample = maxSamples ;

uint16_t *dataPoints = DSOADC::adcInternalBuffer;



// End of DMA indication
volatile static bool dma1_ch1_Active;
#define ADC_CR1_FASTINT 0x70000 // Fast interleave mode DUAL MODE bits 19-16


void snapshotRegisters()
{
        __IO uint32_t *xadc=(uint32_t *)ADC1->regs;
        __IO uint32_t *xadc2=(uint32_t *)ADC2->regs;
        
        for(int i=0;i<10;i++)
        {
            reg1[i]=xadc[i];
            reg2[i]=xadc2[i];
        }
        
}


void zsetup()
{
  adc_calibrate(ADC1);
  adc_calibrate(ADC2);
  setADCs (); //Setup ADC peripherals for interleaved continuous mode.

  // Set up our sensor pin(s)
  pinMode(analogInPin, INPUT_ANALOG);
}

void zloop()
{
  
      // Take our samples
      takeSamples();
      
}
void setADCs ()
{
  //  const adc_dev *dev = PIN_MAP[analogInPin].adc_device;
  int pinMapADCin = PIN_MAP[analogInPin].adc_channel;
  adc_set_sample_rate(ADC1, ADC_SMPR_1_5); //=0,58uS/sample.  ADC_SMPR_13_5 = 1.08uS - use this one if Rin>10Kohm,
  adc_set_sample_rate(ADC2, ADC_SMPR_1_5);    // if not may get some sporadic noise. see datasheet.

  //  adc_reg_map *regs = dev->regs;
  adc_set_reg_seqlen(ADC1, 1);
  ADC1->regs->SQR3 = pinMapADCin;
  ADC1->regs->CR2 |= ADC_CR2_CONT; // | ADC_CR2_DMA; // Set continuous mode and DMA
  ADC1->regs->CR1 |= ADC_CR1_FASTINT; // Interleaved mode
  ADC1->regs->CR2 |= ADC_CR2_SWSTART;

  ADC2->regs->CR2 |= ADC_CR2_CONT; // ADC 2 continuos
  ADC2->regs->SQR3 = pinMapADCin;
}

// Grab the samples from the ADC
// Theoretically the ADC can not go any faster than this.
//
// According to specs, when using 72Mhz on the MCU main clock,the fastest ADC capture time is 1.17 uS. As we use 2 ADCs we get double the captures, so .58 uS, which is the times we get with ADC_SMPR_1_5.
// I think we have reached the speed limit of the chip, now all we can do is improve accuracy.
// See; http://stm32duino.com/viewtopic.php?f=19&t=107&p=1202#p1194

void takeSamples ()
{
  // This loop uses dual interleaved mode to get the best performance out of the ADCs
  //

  dma_init(DMA1);
  dma_attach_interrupt(DMA1, DMA_CH1, xDMA1_CH1_Event);

  adc_dma_enable(ADC1);
  dma_setup_transfer(DMA1, DMA_CH1, &ADC1->regs->DR, DMA_SIZE_32BITS,
                     (uint32_t *)dataPoints, DMA_SIZE_32BITS, (DMA_MINC_MODE | DMA_TRNS_CMPLT));// Receive buffer DMA
  dma_set_num_transfers(DMA1, DMA_CH1, maxSamples / 2);
  dma1_ch1_Active = 1;
  //  regs->CR2 |= ADC_CR2_SWSTART; //moved to setADC
  snapshotRegisters();
  dma_enable(DMA1, DMA_CH1); // Enable the channel and start the transfer.
  //adc_calibrate(ADC1);
  //adc_calibrate(ADC2);
  samplingTime = micros();
  while (dma1_ch1_Active);
  samplingTime = (micros() - samplingTime);

  dma_disable(DMA1, DMA_CH1); //End of trasfer, disable DMA and Continuous mode.
  // regs->CR2 &= ~ADC_CR2_CONT;

}


uint16 timer_set_period(HardwareTimer timer, uint32 microseconds) {
  if (!microseconds) {
    timer.setPrescaleFactor(1);
    timer.setOverflow(1);
    return timer.getOverflow();
  }

  uint32 cycles = microseconds * (72000000 / 1000000); // 72 cycles per microsecond

  uint16 ps = (uint16)((cycles >> 16) + 1);
  timer.setPrescaleFactor(ps);
  timer.setOverflow((cycles / ps) - 1 );
  return timer.getOverflow();
}

/**
* @brief Enable DMA requests
* @param dev ADC device on which to enable DMA requests
*/

void adc_dma_enable(const adc_dev * dev) {
  bb_peri_set_bit(&dev->regs->CR2, ADC_CR2_DMA_BIT, 1);
}


/**
* @brief Disable DMA requests
* @param dev ADC device on which to disable DMA requests
*/

void adc_dma_disable(const adc_dev * dev) {
  bb_peri_set_bit(&dev->regs->CR2, ADC_CR2_DMA_BIT, 0);
}

void xDMA1_CH1_Event() {
  dma1_ch1_Active = 0;
}
// EOF    
