//
#include "lnArduino.h"

#ifdef USE_SMALLFOOTPRINT
  #include "assets/gfx/generated/splash_small_compressed.h"
  const uint8_t *getSplash()
  {
      return splash_small;
  }
  
#else  
  #include "assets/gfx/generated/splash_compressed.h"
  const uint8_t *getSplash()
  {
      return splash;
  }
  
#endif
