
/********************************************************
**  Download from:                                     **
**  http://www.arduino-projekte.de                     **
**                                                     **
**  Based on Code from:				       **
**  http://arduino.cc/playground/                      **
**                                                     **
**  Released into the public domain.                   **
********************************************************/

#ifndef AH_24Cxx_h
#define AH_24Cxx_h

#include <Arduino.h>   //Arduino IDE >= V1.0


#define AT24C01  0
#define AT24C02  1
#define AT24C04  2
#define AT24C08  3
#define AT24C16  4
#define AT24C32  5
#define AT24C64  6
#define AT24C128 8
#define AT24C256 9

class AH_24Cxx
{
  public:
    // Constructor
    AH_24Cxx(WireBase &w,int ic_type,int deviceaddress);   	
    void write_byte  (unsigned int eeaddress, byte data);
    void write_page  (unsigned int eeaddresspage, byte* data, byte length);
    byte read_byte   (unsigned int eeaddress);
    int  read_buffer (unsigned int eeaddress, byte *buffer, int length);

  private:
    int _mode;
    int _deviceaddress;
    
    WireBase &_wire;
};

#endif 
