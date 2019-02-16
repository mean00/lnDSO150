
/********************************************************
**  Download from:                                     **
**  http://www.arduino-projekte.de                     **
**                                                     **
**  Based on Code from:				       **
**  http://arduino.cc/playground/                      **
**                                                     **
**  Released into the public domain.                   **
********************************************************/

#include <Wire.h> 
#include <AH_24Cxx.h>




//************************************************************************

// Constructor
AH_24Cxx::AH_24Cxx(WireBase &w,int ic_type, int deviceaddress) : _wire(w)
{	
  //  _wire.begin(); // initialise the connection
   _mode = ic_type;
   _deviceaddress = B01010<<3 | deviceaddress; //Address: B01010(A2)(A1)(A0)    A2=0; A1=0; A0=0 => Address: B01010000(BIN) | 0x50(HEX) | 80(DEC)
}


//*************************************************************************

  // WARNING: address is a page address, 6-bit end will wrap around
  // also, data can be maximum of about 30 bytes, because the Wire library has a buffer of 32 bytes

  void AH_24Cxx::write_page( unsigned int eeaddresspage, byte* data, byte length ) {

    _wire.beginTransmission(_deviceaddress);
    if (_mode>4){
      _wire.write((int)(eeaddresspage >> 8)); // MSB
      _wire.write((int)(eeaddresspage & 0xFF)); // LSB
    }
    else { 
      _wire.write((int)eeaddresspage);
    }

    byte c;
    for ( c = 0; c < length; c++)
      _wire.write(data[c]);
    _wire.endTransmission();
  }


//*************************************************************************

  // maybe let's not read more than 30 or 32 bytes at a time!

  int AH_24Cxx::read_buffer(unsigned int eeaddress, byte *buffer, int length ) 
  {
    _wire.beginTransmission(_deviceaddress);
    if (_mode>4){
      _wire.write((int)(eeaddress >> 8)); // MSB
      _wire.write((int)(eeaddress & 0xFF)); // LSB
    }
    else { 
      _wire.write((int)eeaddress);
    }
    _wire.endTransmission();
    _wire.requestFrom(_deviceaddress,length);
    int c = 0;
    for ( c = 0; c < length; c++ )
      if (_wire.available()) buffer[c] = _wire.read();
    return length;
  }


//*************************************************************************

  void AH_24Cxx::write_byte(unsigned int eeaddress, byte data) {

    int rdata = data;
    _wire.beginTransmission(_deviceaddress);
    if (_mode>4){
      _wire.write((int)(eeaddress >> 8)); // MSB
      _wire.write((int)(eeaddress & 0xFF)); // LSB
    }
    else { 
      _wire.write((int)eeaddress);
    }
    _wire.write(rdata);
    _wire.endTransmission();
  }

//*************************************************************************

  byte AH_24Cxx::read_byte(unsigned int eeaddress) {
    byte rdata = 0xFF;
    _wire.beginTransmission(_deviceaddress);
    if (_mode>4){
      _wire.write((int)(eeaddress >> 8)); // MSB
      _wire.write((int)(eeaddress & 0xFF)); // LSB
    }
    else { 
      _wire.write((int)eeaddress);
    }
    _wire.endTransmission();
    _wire.requestFrom(_deviceaddress,1);
    if (_wire.available()) rdata = _wire.read();
    return rdata;
  }

