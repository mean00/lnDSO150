#include "lnArduino.h"
#include "gfxfont.h"
#include "Fonts/waree9.h"
//#include "Fonts/waree12.h"
#include "Fonts/OpenSans_Regular14pt7b.h"
//#include "Fonts/ostrich_regular14pt7b.h"
const GFXfont *smallFont()
{
    return &Waree9pt7b;
}
const GFXfont *mediumFont()
{
    return &OpenSans_Regular14pt7b;
}
const GFXfont *bigFont()
{
    return mediumFont();
}
