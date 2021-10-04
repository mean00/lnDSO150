#include "lnArduino.h"
#include "gd32_8bits.h"
#include "lnStopWatch.h"
#include "Fonts/FreeSansBold12pt7b.h"
#include "gfx/dso150nb_compressed.h"
#include "dso_gfx.h"
#include "dso_menuEngine.h"
#include "dso_display.h"

extern void  menuManagement(void);

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
    ili->setFontFamily(&FreeSansBold12pt7b,&FreeSansBold12pt7b,&FreeSansBold12pt7b);
    ili->setFontSize(ili9341::SmallFont);
    ili->setTextColor(GREEN,BLACK);
    
    DSO_GFX::init(ili);
    DSODisplay::init(ili);
    
    
    
    DSODisplay::drawGrid();
    DSODisplay::drawStatsBackGround();
    while(1)
    {
        xDelay(100);
    }
    
    
    lnCycleClock clk;
    
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
