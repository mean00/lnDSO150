
#include "ili_ray.h"
#include "raylib.h"
//
#define WW 320
#define HH 240
Image myBitmap;
Texture2D myTexture;

struct ray_struct
{
    uint32_t x1;
    uint32_t y1;
    uint32_t x2;
    uint32_t y2;
    uint32_t x;
    uint32_t y;
};
ray_struct s;

#define HEY2() printf("Entering %s\n", __FUNCTION__)
#define HEY()                                                                                                          \
    {                                                                                                                  \
    }
void rayIli::next()
{
    s.x += 1;
    if (s.x > s.x2)
    {
        s.x = s.x1;
        s.y += 1;
    }
    if (s.y > s.y2)
    {
        s.y = s.y1;
    }
}
//
rayIli::rayIli(int width, int height) : ili9341(width, height)
{
    xAssert(width == WW);
    xAssert(height == HH);
    myBitmap = GenImageColor(WW * 2, HH * 2, BLACK);
    myTexture = LoadTextureFromImage(myBitmap);
}
//
void rayIli::sendByte(int byte)
{
} // 8 bytes
void rayIli::sendWord_i(int byte)
{
    uint32_t b = (byte >> 11) & 0x1f;
    uint32_t g = (byte >> 5) & 0x3f;
    uint32_t r = (byte >> 0) & 0x1f;

    uint32_t ix = s.x * 2;
    uint32_t iy = s.y * 2;

    Color myColor = {(uint8_t)(r << 3), (uint8_t)(g << 2), (uint8_t)(b << 3), 255};

    Color *pixel = (Color *)myBitmap.data;
    pixel += iy * WW * 2 + ix;
    pixel[0] = myColor;
    pixel[1] = myColor;
    pixel += 2 * WW;
    pixel[0] = myColor;
    pixel[1] = myColor;
    next();
}
void rayIli::sendWord(int byte)
{
    sendWord_i(byte);
}
void rayIli::sendBytes(int nb, const uint8_t *data)
{
    HEY();
}
void rayIli::sendWords(int nb, const uint16_t *data)
{
    HEY();
    for (int i = 0; i < nb; i++)
    {
        sendWord_i(data[i]);
    }
}
void rayIli::updateHwRotation()
{
}
void rayIli::floodWords(int nb, const uint16_t data)
{
    HEY();
    // printf("color = %x\n", data);
    for (int i = 0; i < nb; i++)
        sendWord_i(data);
}
void rayIli::setAddress(int x, int y, int w, int h)
{
    HEY();
    s.x1 = x;
    s.x2 = x + w - 1;
    s.y1 = y;
    s.y2 = y + h - 1;
    s.x = s.x1;
    s.y = s.y1;
}
void rayIli::dataEnd()
{
    HEY();
}
void rayIli::dataBegin()
{
    HEY();
}
void rayIli::pushColors(int len, uint16_t *data)
{
    sendWords(len, data);
}
rayIli::~rayIli()
{
}
void rayIli::init(const uint8_t *init1, const uint8_t *init2)
{
    HEY();
}
void rayIli::sync()
{
    HEY();
    UpdateTexture(myTexture, myBitmap.data);
    BeginDrawing();
    DrawTexture(myTexture, 0, 0, WHITE);
    // DrawFPS(10, 10);
    EndDrawing();
}
// EOF
