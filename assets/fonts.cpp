#include "lnArduino.h"
#include "pfxfont.h"

#ifdef USE_SMALLFOOTPRINT
#include "Fonts/Arimo_Regular14pt7b_small.h"
#include "Fonts/waree9_small.h"
#else
#include "Fonts/Arimo_Regular14pt7b.h"
#include "Fonts/waree9.h"
#endif

const GFXfont *smallFont()
{
    return &Waree9pt7b;
}
const GFXfont *mediumFont()
{
    // return &OpenSans_Regular14pt7b;
    return &Arimo_Regular14pt7b;
}
const GFXfont *bigFont()
{
    return mediumFont();
}
