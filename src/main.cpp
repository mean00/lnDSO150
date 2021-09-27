#include "lnArduino.h"
#include "gd32_8bits.h"
void setup()
{

}
uint32_t chipId;
void loop()
{

    ln8bit9341 *ili=new ln8bit9341( 320,240, 
                                    1,          // port B
                                    PC14,       // DC/RS
                                    PC13,       // CS
                                    PC15,       // Write
                                    PA6,        // Read
                                    PB9);       // LCD RESET
    ili->init();
    chipId=ili->readChipId();
    Logger("Chip Id = %d\n",chipId);
//    ili->square(10,10,60,60,0xfff);
    while(1)
    {
        xDelay(100);
    }

}
