#include "esprit.h"
#include "pfxfont.h"

#define PROGMEM

#ifdef USE_SMALLFOOTPRINT
#include "Fonts/Arimo_Regular14pt7b_small.h"
#include "Fonts/wareeb9_small.h"
#define waree_bold9pt7b waree_bold9pt7b
#else
#include "Fonts/Arimo_Regular14pt7b.h"
#include "Fonts/wareeb9.h"
#endif

const GFXfont *smallFont()
{
    return &waree_bold9pt7b; //
}
const GFXfont *mediumFont()
{
    // return &OpenSans_Regular14pt7b;
    return &Arimo_Bold14pt7b;
}
const GFXfont *bigFont()
{
    return mediumFont();
}
