/***************************************************
 STM32 duino based firmware for DSO SHELL/150
 *  * GPL v2
 * (c) mean 2019 fixounet@free.fr
 ****************************************************/

#pragma once
#include "lnArduino.h"
enum DSO_ArmingMode
{
    DSO_CAPTURE_SINGLE = 0,
    DSO_CAPTURE_MULTI,
    DSO_CAPTURE_MODE_INVALIDE
};

class ili9341;
/**
 */
class DSODisplay
{
  public:
    /**
     */
    typedef enum
    {
        INVALID_MODE = 0,
        VOLTAGE_MODE = 1,
        TIME_MODE = 2,
        TRIGGER_MODE = 3,
        ARMING_MODE = 4,
        STAT_MODE = 5,
        VOLTAGE_OFFSET_MODE = VOLTAGE_MODE + 0x80,
        TIME_MODE_ALT = TIME_MODE + 0x80,
        TRIGGER_MODE_ALT = TRIGGER_MODE + 0x80,
    } MODE_TYPE;

  public:
  public:
    static void init(ili9341 *d);
    static void cleanup();

    static void drawWaveForm(int count, const uint8_t *data);
    static void drawMinMax(float mn, float mx);
    static void drawFq(int fq);
    static void drawGrid(void);
    // individual item on the screen
    static void drawVolt(const char *v, bool highlight);
    static void drawTrigger(const char *v, bool highlight);
    static void drawTime(const char *v, bool highlight);
    static void drawArming(const char *v, bool highlight);
    static void drawCoupling(const char *v, bool highlight);
    // individual item on the screen
    static void drawVerticalTrigger(bool drawOrErase, int column);
    static void drawVoltageTrigger(bool drawOrErase, int line);
    static void drawStatsBackGround();
    static void drawMode(MODE_TYPE mode);
    static MODE_TYPE getMode();
    static void setMode(MODE_TYPE t);
    static void printTriggerValue(float volt, bool hilight = false);
    static void printOffsetValue(float volt, bool hilight = false);

    static void drawAutoSetup();
    static void drawAutoSetupStep(int x);
    static void drawTriggered(bool triggered);

    static void drawSplash();
    static void printString(int x, int y, const char *text);
    static void clearScreen();
};

#define DSO_WAVEFORM_WIDTH 240  // 20 pixel / unit, 12 square => 240 pixel
#define DSO_WAVEFORM_HEIGHT 200 // 10 square : 200 pixel
#define DSO_WAVEFORM_OFFSET ((240 - DSO_WAVEFORM_HEIGHT) / 4)
