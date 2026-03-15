#pragma once
#include "esprit.h"
//
#include "simpler9341.h"
//
//
class rayIli : public ili9341
{
  public:
    rayIli(int width, int height);
    virtual ~rayIli();
    //---------------------------------------------------------------
    virtual void sendByte(int byte);                      // 8 bytes
    virtual void sendWord(int byte);                      // 16 bytes
    virtual void sendBytes(int nb, const uint8_t *data);  // 8 bits
    virtual void sendWords(int nb, const uint16_t *data); // 16 bits, that one must be inside dataBegin() dataEnd()
    virtual void updateHwRotation();
    virtual void floodWords(int nb, const uint16_t data); // 16 bits
    virtual void setAddress(int x, int y, int w, int h);
    virtual void dataEnd();
    virtual void dataBegin();
    virtual void pushColors(int len, uint16_t *data);
    virtual void init(const uint8_t *init1, const uint8_t *init2);

    //
    void next();
    void sendWord_i(int byte);
    void sync();
};
