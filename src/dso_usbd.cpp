#include "lnArduino.h"
#include "dso_version.h"
#include "include/lnUsbStack.h"
#include "include/lnUsbCDC.h"
#include "cdc_descriptor.h"
#include "include/lnUsbDFUrt.h"


#include "dso_usbd_api.h"

#include "pb_common.h"
#include "pb_encode.h"
#include "pb_decode.h"

#include "messaging.pb.h"

lnUsbStack *usb =NULL;
lnUsbCDC *cdc=NULL;
#define MEVENT(x)        case lnUsbStack::USB_##x: Logger(#x); break;
void message_received(int size,const uint8_t *data);
/**
*/

#define PROLOG()   Logger("Running automaton : State = %d, val=0x%x\n",_state,c);

#define EPILOG() goto again;

#define PB_BUFFER_SIZE 64
/**
  This is a simple framer that extracts frames
  from a CDC stream
  It is reasonnably robust
   The format is 
    "DO" => connect
        <= "OD"

    then for each frame ...
    "S" => start of frame
      Size = 16 bits, LSB
      [...] = data
    "E" => end of frame
*/
const uint8_t connect_reply[2]={'O','D'};
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
      /*
      */
      bool send_message(int sz, const uint8_t *d)
      {
        uint8_t tmp[3]={'S',(uint8_t )(sz&0xff), (uint8_t )(sz>>8)};
        if(3!=_cdc->write(tmp,3))
        {
          return false;
        }
        while(sz)
        {
          int r=_cdc->write(d,sz);
          if(r)
          {
            d+=r;
            sz-=r;
          }else
          {
            Logger("Cannot write!\n");
            return false;
          }
        }
        // send tail
        const uint8_t tail = 'E';
        if(1!=_cdc->write(&tail,1))
        {
          return false;
        }
        _cdc->flush();
        return true;
      }
      /*
      */
      void process_data()
      {
    again:
        
        uint8_t c; 
        int n=_cdc->read(&c,1); 
        if(!n) return;
        switch(_state)
        {
          case STATE_BEGIN:
                {
                  PROLOG()
                  if(c=='D')
                  {
                    _state=STATE_BEGIN2;
                  }else
                  {
                    Logger("Wrong handshake :%x\n",c);
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
                    _cdc->write(connect_reply,2);
                     _cdc->flush();
                  }else
                  {
                    if(c=='D')
                      _state = STATE_BEGIN2;
                    else
                      _state = STATE_BEGIN;
                  }
                  EPILOG()
                }
                break;     
          case STATE_HEAD:
          {
                  PROLOG()
                  _size=0;
                  if(c=='S')
                  {
                    _state= STATE_SIZE1;                     
                  }else
                  {
                    Logger("Wrong head :%x\n",c);
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
                    Logger("Overflow\n");
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
                      message_received(_dex, _buffer);
                  }else
                  {
                      Logger("BadTail");
                  }                  
                  _state=STATE_HEAD;
                  EPILOG()
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

    DSO_API::init();

    usb->start();
}

static void send_reply(const UnionMessage &msg)
{
  uint8_t buffer[PB_BUFFER_SIZE];
  pb_ostream_t o = pb_ostream_from_buffer(buffer, PB_BUFFER_SIZE);
  if(false== pb_encode(&o, &UnionMessage_msg, &msg))
  {
    Logger("Failed to encode\n");
  }
  if(automaton)
  {
    Logger("Reply sent\n");
    automaton->send_message(o.bytes_written, buffer);
  }
}

void rusb_reply(bool reply)
{
  UnionMessage msg;
  msg.which_msg = UnionMessage_msg_r_tag;
  if(reply)
    msg.msg.msg_r.s = STATUS_OK;
  else
    msg.msg.msg_r.s = STATUS_KO;
  send_reply(msg);
}

/**
*/
void message_received(int size,const uint8_t *data)
{
  // decode..
  UnionMessage msg;
  pb_istream_t s = pb_istream_from_buffer(data, size);
;
  if(!pb_decode(&s, &UnionMessage_msg, &msg))
  {
    Logger("Error decoding pb message!");
    return;
  }
  #define XXX(x)  case UnionMessage_msg_##x##_tag 
  switch(msg.which_msg)
  {
    XXX(sv):  // voltage
            rusb_reply( DSO_API::setVoltage( msg.msg.msg_sv.voltage) );
            break;
    XXX(stb): // timebase
            rusb_reply( DSO_API::setTimeBase( msg.msg.msg_stb.timebase) );
            break;
    XXX(str): // trigger
            rusb_reply( DSO_API::setTrigger( msg.msg.msg_str.trigger) );
            break;

#define NNREPLY(api, tag, field, type)            \
                int voltage = DSO_API::api; \
                UnionMessage msg; \
                msg.which_msg = UnionMessage_msg_##tag##_tag; \
                msg.msg.msg_##tag.field=(type)voltage; \
                send_reply(msg); 

    XXX(gv):  // voltage
            {
              NNREPLY(getVoltage() , sv, voltage , VOLTAGE);
            }
            break;
    XXX(gtb): // timebase
          {
              NNREPLY(getTimeBase() , stb, timebase , TIMEBASE);
            }    
            break;
    XXX(gtr): // trigger
            {
              NNREPLY(getTrigger() , str, trigger , TRIGGER);
            }    
            break;

    default:
        Logger("Unknown message received");
        break;
  }
}
// EOF

