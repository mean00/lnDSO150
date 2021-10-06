#include "lnArduino.h"
#include "gd32_8bits.h"
#include "lnStopWatch.h"
#include "assets/gfx/dso150nb_compressed.h"
#include "dso_gfx.h"
#include "dso_menuEngine.h"
#include "dso_display.h"
#include "dso_test_signal.h"
#include "pinConfiguration.h"

extern void  menuManagement(void);
extern const GFXfont *smallFont();
extern const GFXfont *mediumFont();
extern const GFXfont *bigFont();


void setup()
{

}
uint32_t chipId;
void loop()
{
    Logger("Starting DSO...\n");
    ln8bit9341 *ili=new ln8bit9341( 240, 320,
                                    1,          // port B
                                    PC14,       // DC/RS
                                    PC13,       // CS
                                    PC15,       // Write
                                    PA6,        // Read
                                    PB9);       // LCD RESET
    ili->init();    
    ili->setRotation(1);
    
    ili->fillScreen(0);
    ili->setFontFamily(smallFont(),mediumFont(),bigFont());
    ili->setFontSize(ili9341::SmallFont);
    ili->setTextColor(GREEN,BLACK);
    
    DSO_GFX::init(ili);
    DSODisplay::init(ili);
    
    
    
    DSODisplay::drawGrid();
    DSODisplay::drawStatsBackGround();
   
    
    
    lnCycleClock clk;
    int fq[4]={100,1000,10000,100000};
    int fqIndex=0;
    testSignal *ts=new testSignal(PIN_TEST_SIGNAL,PIN_TEST_SIGNAL_AMP);
    
    
    
    
    while(1)
    {
        uint32_t micro=lnGetUs();
        ili->fillScreen(0xffff);
        Logger("FillScreen white %d us \n",lnGetUs()-micro);
        xDelay(1000);
        micro=lnGetUs();
        ili->fillScreen(0x1f);
        Logger("FillScreen rng %d us \n",lnGetUs()-micro);
        xDelay(1000);
        ts->setFrequency(fq[fqIndex]);
        fqIndex=(fqIndex+1)%4;
    }
    
    xDelay(2000);
    ili->fillScreen(0xf);
    xDelay(2000);
    ili->fillScreen(0xf<<5);
    
    while(1)
    {
        clk.restart();
        uint32_t micro=lnGetUs();
        
        ili->print(20,100,"This is a hello world program 123\n");
        micro=lnGetUs()-micro;
        uint32_t calibration=clk.elapsed();
        
        Logger("FillScreen %d ticks, %d us \n",calibration,micro);
        clk.restart();
        xDelay(10);
        calibration=clk.elapsed();
        Logger("10 ms  %d ticks\n",calibration);            
    }
    
    clk.restart();
    xDelay(10);
    clk.restart();
    xDelay(10);
    uint32_t calibration=clk.elapsed();
    Logger("10ms is %d ticks\n",calibration);
    uint32_t bf;
    
  //  menuManagement();
    
   
    while(1)
    {
        bf=lnGetUs();
        //ili->fillScreen(1*0x1f);
          Logger("Sending Hello\n");
          xDelay(2000);

        ili->print(10,60,"Hi there, this is hello world");
        //ili->drawRLEBitmap(96,96,60,60,0xfff,0,dso150nb); //7.5 ms
        Logger("Fill Screen : %d us \n",lnGetUs()-bf);
    }
    while(1)
    {
        xDelay(100);
    }

}
