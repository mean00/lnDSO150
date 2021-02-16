#pragma once
#include "MapleFreeRTOS1000_pp.h"
#include "USBComposite.h"
#include "USBCompositeSerial.h"
/**
 * 
 */
class UsbTask : public xTask
{
public:
    enum SerialState
    {
        Disconnected=0,
        Connected=1,
        Handshaking=2
    };
                UsbTask(const char *name,  int priority=2, int taskSize=100): xTask(name,priority,taskSize)
                {
                    _connected=Disconnected;
                }
        void    run();
        void    write32(uint32_t v);
        virtual void    processCommand(uint32_t command)=0;

        
protected:
    
        SerialState    _connected;
        
};