#include "dso_debug.h"
#include "dso_usb.h"

USBCompositeSerial CompositeSerial;



#define MKFCC(a,b,c,d) ( (a<<24)+(b<<16)+(c<<8)+d)
/**
 * 
 */
void UsbTask::run()
{
    uint32_t magicWord;
    int      magicCount;
    while(1)
    {
        switch(_connected)
        {
            case Disconnected:
                if(CompositeSerial.isConnected())
                {
                    _connected=Handshaking;
                    magicWord=0;
                    Logger("Plugged\n");
                    continue;
                }
                xDelay(100);
                break;     
            case Handshaking:
                if(!CompositeSerial.isConnected()) {   _connected=Disconnected;Logger("Disconnected\n");continue;   }
                if(CompositeSerial.available())
                {
                    int n=CompositeSerial.read();
                    magicWord=(magicWord<<8)+n;
                    if(magicWord==MKFCC('D','S','O','0'))
                    {
                        write32(MKFCC('O','S','D','0'));
                         Logger("Connected\n");
                        _connected=Connected;
                        magicWord=0;
                        magicCount=0;
                        continue;
                    }
                }
                xDelay(100);
                break;
            case Connected:
                if(!CompositeSerial.isConnected()) {   _connected=Disconnected;Logger("Disconnected\n");continue;   }
                
                if(CompositeSerial.available())
                {
                    int n=CompositeSerial.read();
                    magicWord=(magicWord<<8)+n;
                    magicCount++;
                    if(magicCount==4)
                    {
                        processCommand(magicWord);
                        magicWord=0;
                        magicCount=0;
                    }
                    
                }
                
                xDelay(100);
                break;
            default:
                xAssert(0);
                break;
        }
    }
}
/**
 * 
 * @param v
 */
void UsbTask::write32(uint32_t v)
{
    uint8_t c[4];
    c[0]=v>>24;
    c[1]=v>>16;
    c[2]=v>>8;
    c[3]=v;
    CompositeSerial.write(c,4);
}
void UsbTask::writeFloat(const float f)
{
    uint8_t *c=(uint8_t *)&f;   
    CompositeSerial.write(c,4);
}

// EOF