#include "lnArduino.h"
#include "ili_ln8bits.h"
#include "lnStopWatch.h"
#include "dso_gfx.h"
#include "dso_menuEngine.h"
#include "dso_display.h"
#include "dso_test_signal.h"
#include "pinConfiguration.h"
#include "dso_control.h"
#include "DSO_portBArbitrer.h"
#include "dso_calibrate.h"
#include "gd32/nvm_gd32.h"
#include "dso_adc_gain.h"
#include "dso_capture.h"
#include "lnCpuID.h"

void testFunc()
{
        DSODisplay::clearScreen();
        DSODisplay::printString(8, 64,"abcdefghijklmnop rstuvwxyz0123456789");          

//        while(1)
        {
        }
}
