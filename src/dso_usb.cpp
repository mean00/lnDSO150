#include "USBComposite.h"
#include "USBCompositeSerial.h"
/**
 * 
 */
USBCompositeSerial CompositeSerial;
extern "C" void dsoUsb_init()
{
    USBComposite.clear();
    CompositeSerial.registerComponent();
    USBComposite.begin();
}