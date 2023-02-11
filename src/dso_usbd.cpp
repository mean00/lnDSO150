#include "lnArduino.h"
#include "dso_version.h"
#include "include/lnUsbStack.h"
#include "include/lnUsbCDC.h"
#include "cdc_descriptor.h"
#include "include/lnUsbDFUrt.h"


lnUsbStack *usb =NULL;
lnUsbCDC *cdc=NULL;
#define MEVENT(x)        case lnUsbStack::USB_##x: Logger(#x); break;

void processUsbEvent()
{
  
}
/**

  The format is 
    "DO" => connect
    "S" => start of frame
      Size = 16 bits, LSB
      [...] = data
    "E" => end of frame

*/
#define PROLOG()  uint8_t c; \
                  int n=_cdc->read(&c,1); \
                  if(!n) return;

#define EPILOG() goto again;

#define PB_BUFFER_SIZE 64

class usb_automaton
{
  public:
      enum USB_COMMUNICATION_STATE
      {
        STATE_BEGIN,
        STATE_BEGIN2,
        STATE_HEAD,
        STATE_SIZE1,
        STATE_SIZE2,
        STATE_BODY,
        STATE_TAIL,
      };
  public:
      usb_automaton(lnUsbCDC *cdc)
      {
        _cdc=cdc;
        _state = STATE_BEGIN;
      }
      void process_data()
      {
        again:
        switch(_state)
        {
          case STATE_BEGIN:
                {
                  PROLOG()
                  if(c=='D')
                  {
                    _state=STATE_BEGIN2;
                  }
                  EPILOG()
                }
                break;
          case STATE_BEGIN2:
                {
                  PROLOG()
                  if( c=='O')
                  {
                    Logger("Connected\n");
                    _state=STATE_HEAD;
                  }else
                  {
                    _state = STATE_BEGIN;
                  }
                  EPILOG()
                }
                break;     
          case STATE_HEAD:
          {
                  PROLOG()
                  if(c=='S')
                  {
                    _state= STATE_SIZE1;                     
                  }
                  EPILOG()
                  break;
          }
          case STATE_SIZE1:
          {
                  PROLOG()
                  _size=c;
                  _state= STATE_SIZE2;
                  EPILOG()
                  break;
          }
          case STATE_SIZE2:
          {
                  PROLOG()
                  _size=_size+(c<<8);
                  _dex=0;
                  _state= STATE_BODY;
                  EPILOG()
                  break;
          }
          case STATE_BODY:
          {
                  PROLOG()
                  _buffer[_dex++]=c;
                  if(_dex > PB_BUFFER_SIZE)
                  {
                      _state=STATE_HEAD;                      
                  }
                  if(_dex==_size)
                  {
                    _state=STATE_TAIL;
                  }
                  EPILOG()
                  break;
          }
          case STATE_TAIL:
          {
                  PROLOG()
                  if(c=='E')
                  {
                      Logger("Got command of %d bytes\n",_size);
                  }else
                  {
                      Logger("BadTail");
                  }                  
                  _state=STATE_HEAD;
                  goto again;
          }
          break;
          
          default:
              xAssert(0);                           
        }
      }
protected:
    USB_COMMUNICATION_STATE _state;
    lnUsbCDC                *_cdc;
    int                      _size;
    int                      _dex;
    uint8_t                  _buffer[PB_BUFFER_SIZE];
};

usb_automaton *automaton=NULL;

/**
*/
void dsoUsbEvent(void *cookie, lnUsbStack::lnUsbStackEvents event)
{
    switch (event)
    {
        MEVENT(CONNECT)
        MEVENT(DISCONNECT)
        MEVENT(SUSPEND)
        MEVENT(RESUME)
        default: xAssert(0); break;
    }
}
extern void lnSoftSystemReset(void);
extern void lnHardSystemReset(void);

void goDfu()
{
  Logger("Rebooting to DFU...\n");
  // pull DP to low
  lnDigitalWrite(PA12,0);
  lnPinMode(PA12,lnOUTPUT);
  lnDelayMs(100);
  volatile uint32_t *ram=(volatile uint32_t *)  0x20000000;

  ram[0]=0xCC00FFEEUL;
  ram[1]=0xDEADBEEFUL;
  lnHardSystemReset(); 
  //lnSoftSystemReset();
}

/**
*/
void cdcEventHandler(void *cookie,int interface,lnUsbCDC::lnUsbCDCEvents event,uint32_t payload)
{
  uint8_t buffer[32];

    switch (event)
    {
      case lnUsbCDC::CDC_DATA_AVAILABLE:
          automaton->process_data();
          break;
      case lnUsbCDC::CDC_SESSION_START:
          Logger("CDC SESSION START\n");  
           automaton=new usb_automaton(cdc);
          break;
      case lnUsbCDC::CDC_SESSION_END:
          Logger("CDC SESSION END\n");
          cdc->clear_input_buffers();
          delete automaton;
          automaton = NULL;
          break;
      case lnUsbCDC::CDC_SET_SPEED:
          break;
      default:
          xAssert(0);
          break;
    }
}

/**
*/
void dsoInitUsb()
{
    usb = new lnUsbStack;
    usb->init(5, device_descriptor);
    usb->setConfiguration(desc_hs_configuration, desc_fs_configuration, &desc_device, &desc_device_qualifier);
    usb->setEventHandler(NULL, dsoUsbEvent);

    cdc=new lnUsbCDC(0);
    cdc->setEventHandler(cdcEventHandler,NULL);
    lnUsbDFURT::addDFURTCb(goDfu);
    usb->start();
}

// EOF
