#pragma once

class tusb_desc_device_qualifier_t
{

};
class tusb_desc_device_t
{

};
extern  const char  **device_descriptor;
extern  tusb_desc_device_qualifier_t  desc_device_qualifier;
extern tusb_desc_device_t desc_device;

extern uint8_t  *desc_fs_configuration;
extern uint8_t *desc_hs_configuration;

class lnUsbStack
{
public:
    enum lnUsbStackEvents
    {
        USB_CONNECT,
        USB_DISCONNECT,
        USB_SUSPEND,
        USB_RESUME
    };
  typedef void lnUsbStackEventHandler(void *cookie, const lnUsbStackEvents event);



public:
    lnUsbStack();   
    void start();
    void stop();
    void setEventHandler(void *cookie, lnUsbStackEventHandler *ev);

    void init(int nbDescriptorLine, const char **deviceDescriptor);
    void setConfiguration(const uint8_t *hsConfiguration, const uint8_t *fsConfiguration,
                        const tusb_desc_device_t *desc, const tusb_desc_device_qualifier_t *qual);


 
};
typedef void lnDFUCb(void);

class  lnUsbDFURT
{
    public:
        static void addDFURTCb(lnDFUCb *fun);
};
