#include "lnArduino.h"
#include "gd32_8bits.h"
#include "lnStopWatch.h"
#include "Fonts/FreeSansBold12pt7b.h"
#include "gfx/dso150nb_compressed.h"
#include "dso_gfx.h"
void setup()
{

}
uint32_t chipId;
void loop()
{

    ln8bit9341 *ili=new ln8bit9341( 240, 320,
                                    1,          // port B
                                    PC14,       // DC/RS
                                    PC13,       // CS
                                    PC15,       // Write
                                    PA6,        // Read
                                    PB9);       // LCD RESET
    ili->init();
    ili->setRotation(1);
    
    DSO_GFX::init(ili);
    
    chipId=ili->readChipId();
    Logger("Chip Id = 0x%x\n",chipId);
    lnCycleClock clk;
    ili->setFontFamily(&FreeSansBold12pt7b,&FreeSansBold12pt7b,&FreeSansBold12pt7b);
    ili->setFontSize(ili9341::SmallFont);
    ili->setTextColor(0xff0,0xf00);
    clk.restart();
    xDelay(10);
    clk.restart();
    xDelay(10);
    uint32_t calibration=clk.elapsed();
    Logger("10ms is %d ticks\n",calibration);
    uint32_t bf;
    
    
   
    while(1)
    {
        bf=lnGetUs();
        //ili->fillScreen(1*0x1f);
        ili->print(10,60,"Hi there, this is hello world");
        //ili->drawRLEBitmap(96,96,60,60,0xfff,0,dso150nb); //7.5 ms
        Logger("Fill Screen : %d us \n",lnGetUs()-bf);
    }
    while(1)
    {
        xDelay(100);
    }

}
