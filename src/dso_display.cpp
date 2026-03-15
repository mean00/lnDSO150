/***************************************************
 STM32 duino based firmware for DSO SHELL/150
 *  * GPL v2
 * (c) mean 2019 fixounet@free.fr
 *
 * /!\ All related to waveform (waveform + trigger) are automatically centered,
 * you dont need to apply offset
 *
 *
 ****************************************************/
#include "dso_display.h"
#include "DSO_portBArbitrer.h"
#include "assets/gfx/generated/splash_decl.h"
#include "dso_colors.h"
#include "dso_version.h"
#include "esprit.h"
#include "lnCpuID.h"
#include "math.h"
#include "pattern_decl.h"
#include "simpler9341.h"

#define XMAX(x, y) (((x) > (y)) ? (x) : (y))
#define XMIN(x, y) (((x) < (y)) ? (x) : (y))

#define LIGHT_ILI_GREEN ILI_MK_COLOR(0, 0x2f, 0) // careful the colors are also in genpattern.py
#define DARK_ILI_GREEN ILI_MK_COLOR(0, 0x1f, 0)

#define AUTOCAL_BOX_WIDTH 200
#define AUTOCAL_BOX_HEIGHT 80
#define AUTOCAL_BORDER 10
#define AUTOCAL_COLOR ILI_WHITE

#define SCALE_STEP 20
#define C_X 12
#define C_Y 10

#define LINE_OFFSET 4

#define DSO_LOW_BAR_BUTTON 68
#define DSO_CHAR_HEIGHT 20
#define DSO_HEIGHT_OFFSET 1
#define DSO_INFO_START_COLUMN (242)
#define DSO_INFO_MAX_WIDTH (320 - DSO_INFO_START_COLUMN - 8)

#define BG_STATS_HEIGHT (240 - 20)
#define BG_COLOR LIGHT_ILI_GREEN

#define LED_X_OFFSET 10
#define LED_Y_OFFSET 4
#define LED_WIDTH ((320 - DSO_INFO_START_COLUMN - LED_X_OFFSET * 2))
#define LED_HEIGHT 12

static void prettyPrint(float x, int mx);

#define TITLE_SQUARE(x, color)                                                                                         \
    tft->square(color, DSO_INFO_START_COLUMN, DSO_HEIGHT_OFFSET + (HeaderLine[x] - 1) * DSO_CHAR_HEIGHT,               \
                320 - DSO_INFO_START_COLUMN, DSO_CHAR_HEIGHT);

static ili9341 *tft;
extern DSO_portArbitrer *arbitrer;
static DSO_ArmingMode oldMode = DSO_CAPTURE_MODE_INVALIDE;
static DSODisplay::MODE_TYPE mode = DSODisplay::VOLTAGE_MODE;

uint8_t prevPos[256];
uint8_t prevSize[256];
static char textBuffer[24];

extern const uint8_t *getSplash();

#define MIN_ROW 0
#define MAX_ROW 1
#define FREQ_ROW 2
#define TRIGGER_ROW 3
#define VOLTAGE_OFFSET_ROW 4
#define ARMING_ROW 5

#define RIGHT_HAND_OFFSET 1

static const int DisplayLine[10] = {
    RIGHT_HAND_OFFSET + 2, // MIN_ROW
    RIGHT_HAND_OFFSET + 3, // MAX_ROW
    RIGHT_HAND_OFFSET + 5, // FREQ_ROW
    RIGHT_HAND_OFFSET + 7, // TRIGGER_ROW
    RIGHT_HAND_OFFSET + 9, // VOLTAGE_OFFSET_ROW
};
static const int HeaderLine[10] = {
    RIGHT_HAND_OFFSET + 1,  // MIN_ROW
    RIGHT_HAND_OFFSET + 1,  // MAX_ROW
    RIGHT_HAND_OFFSET + 4,  // FREQ_ROW
    RIGHT_HAND_OFFSET + 6,  // TRIGGER_ROW
    RIGHT_HAND_OFFSET + 8,  // VOLTAGE_OFFSET_ROW
    RIGHT_HAND_OFFSET + 10, // ARMING_ROW
};

class AutoGfx
{
  public:
    AutoGfx()
    {
        arbitrer->beginLCD();
    }
    ~AutoGfx()
    {
        arbitrer->endLCD();
    }
};
const char *getTextBuffer()
{
    return textBuffer;
}
// ---------------------------------------------------------------------------
// Number formatting
// ---------------------------------------------------------------------------

/**
 * Format a float as "[-]ddd.dd" with up to post_digit fractional digits
 * into the global textBuffer.
 */
void format_float_02_2f(float value, int post_digit)
{
    char *buf = textBuffer;
    char *limit = textBuffer + sizeof(textBuffer) - 1; // reserve one slot for '\0'

    auto emit = [&](char c) {
        if (buf < limit)
            *buf++ = c;
    };
    auto check = [&]() -> bool {
        if (buf >= limit)
        {
            *buf = '\0';
            return true;
        }
        return false;
    };

    if (value < 0)
    {
        emit('-');
        value = -value;
    }

    value += 0.005f;
    int int_part = (int)value;
    int frac_part = (int)((value - (float)int_part) * 100.f);
    if (frac_part >= 100)
    {
        frac_part = 0;
        int_part++;
    }

    // Integer part
    if (int_part == 0)
    {
        emit('0');
        emit('0');
    }
    else
    {
        bool started = false;
        static const int dividers[3] = {1000, 100, 10};
        for (int i = 0; i < 3; i++)
        {
            const int divisor = dividers[i];
            if (int_part >= divisor)
                started = true;
            if (started)
            {
                int digit = int_part / divisor;
                emit('0' + digit);
                int_part -= digit * divisor;
            }
            // Only check after 1000s and 100s columns; the 10s pass falls
            // through so the units digit below is always paired with it.
            if (i < 2 && check())
                return;
        }
        emit('0' + int_part); // units digit
    }

    if (check())
        return;
    emit('.');
    if (check())
        return;

    // Fractional part
    int_part = frac_part;
    int tens = int_part / 10;
    emit('0' + tens);
    int_part -= tens * 10;
    if (post_digit >= 2)
        emit('0' + int_part);

    // Always null-terminate (limit was reserved for this).
    *buf = '\0';
}

static void VALUE_Y_POSITION(int x)
{
    tft->setCursor(DSO_INFO_START_COLUMN + 2, DSO_HEIGHT_OFFSET + DisplayLine[x] * DSO_CHAR_HEIGHT - 5);
}

static const uint16_t *getBackGround(int line, bool &repeat)
{
    repeat = false;
    if (!(line % SCALE_STEP))
    {
        repeat = true;
        return (uint16_t *)darkGreenPattern;
    }
    if (line == DSO_WAVEFORM_WIDTH / 2)
    {
        repeat = true;
        return (uint16_t *)lightGreenPattern;
    }
    return (uint16_t *)defaultPattern;
}

static const uint16_t *getHzBackGround(int line, bool &repeat)
{
    repeat = false;
    if (!(line % SCALE_STEP))
    {
        repeat = true;
        return (uint16_t *)darkGreenPattern;
    }
    if (line == DSO_WAVEFORM_WIDTH / 2)
    {
        repeat = true;
        return (uint16_t *)lightGreenPattern;
    }
    return (uint16_t *)horizontal;
}

static void printMeasurement(int line, float f)
{
    VALUE_Y_POSITION(line);
    prettyPrint(f, 320 - 4 - DSO_INFO_START_COLUMN);
}

static const char *fq2Text(int fq)
{
    static char buff[16];
    float f = (float)fq;
    const char *suff = "";

    if (fq > (1000000 * 11) / 10)
    {
        suff = "M";
        f /= 1000000.f;
    }
    else if (fq > (1000 * 11) / 10)
    {
        suff = "K";
        f /= 1000.f;
    }

    if (*suff)
    {
        format_float_02_2f(f, 1);
        strcat(textBuffer, suff);
        return textBuffer;
    }
    sprintf(buff, "%d", fq);
    return buff;
}

// ---------------------------------------------------------------------------
// DSODisplay methods
// ---------------------------------------------------------------------------

void DSODisplay::init(ili9341 *d)
{
    tft = d;
    for (int i = 0; i < 256; i++)
    {
        prevPos[i] = 120;
        prevSize[i] = 1;
    }
}

void DSODisplay::drawWaveForm(int count, const uint8_t *data)
{
    AutoGfx autogfx;

    if (count < 3)
        return;

    int last = XMIN(data[0], DSO_WAVEFORM_HEIGHT - 1);

    for (int j = 1; j < count - 1; j++)
    {
        int color = ILI_YELLOW;
        int next = data[j];

        if (next == 0)
        {
            next = 1;
            color = ILI_RED;
        }
        if (next > DSO_WAVEFORM_HEIGHT - 1)
        {
            next = DSO_WAVEFORM_HEIGHT - 1;
            color = ILI_RED;
        }

        int start = XMIN(last, next);
        int sz = abs(next - last);
        if (!sz)
            sz = 1;

        bool repeat;
        const uint16_t *bg = getBackGround(j, repeat);

        tft->setAddress(j, prevPos[j] + DSO_WAVEFORM_OFFSET, 1, prevSize[j]);
        if (repeat)
            tft->floodWords(prevSize[j], bg[0]);
        else
            tft->pushColors(prevSize[j], ((uint16_t *)bg) + prevPos[j]);

        tft->VLine(j, start + DSO_WAVEFORM_OFFSET, sz, color);
        prevSize[j] = sz;
        prevPos[j] = start;
        last = next;
    }
}

void DSODisplay::drawMinMax(float mn, float mx)
{
    AutoGfx autogfx;
    tft->setTextColor(ILI_WHITE, ILI_BLACK);
    printMeasurement(MIN_ROW, mn);
    printMeasurement(MAX_ROW, mx);
}

void DSODisplay::drawFq(int f)
{
    AutoGfx autogfx;
    VALUE_Y_POSITION(FREQ_ROW);
    if (f == 0)
    {
        tft->square(0, DSO_INFO_START_COLUMN, DSO_HEIGHT_OFFSET + (DisplayLine[FREQ_ROW] - 1) * DSO_CHAR_HEIGHT,
                    320 - DSO_INFO_START_COLUMN, DSO_CHAR_HEIGHT);
        tft->print("-");
        return;
    }
    tft->setTextColor(ILI_WHITE, ILI_BLACK);
    tft->printUpTo(fq2Text(f), 320 - DSO_INFO_START_COLUMN);
}

void DSODisplay::cleanup()
{
    AutoGfx autogfx;
    tft->setTextColor(LIGHT_ILI_GREEN, 0);
    tft->fillScreen(0);
}

void DSODisplay::drawGrid(void)
{
    AutoGfx autogfx;
    const uint16_t fgColor = DARK_ILI_GREEN;
    const uint16_t hiLight = LIGHT_ILI_GREEN;

    for (int i = 0; i <= C_Y; i++)
        tft->HLine(0, SCALE_STEP * i + DSO_WAVEFORM_OFFSET, SCALE_STEP * C_X, fgColor);
    for (int i = 0; i <= C_X; i++)
        tft->VLine(SCALE_STEP * i, DSO_WAVEFORM_OFFSET, SCALE_STEP * C_Y, fgColor);

    // Border and centre cross in bright green
    tft->HLine(0, DSO_WAVEFORM_OFFSET, SCALE_STEP * C_X, hiLight);
    tft->HLine(0, DSO_WAVEFORM_OFFSET + DSO_WAVEFORM_HEIGHT - 1, SCALE_STEP * C_X, hiLight);
    tft->HLine(0, DSO_WAVEFORM_OFFSET + DSO_WAVEFORM_HEIGHT / 2, SCALE_STEP * C_X, hiLight);
    tft->VLine(0, DSO_WAVEFORM_OFFSET, SCALE_STEP * C_Y, hiLight);
    tft->VLine(C_X * SCALE_STEP, DSO_WAVEFORM_OFFSET, SCALE_STEP * C_Y, hiLight);
    tft->VLine((C_X * SCALE_STEP) / 2, DSO_WAVEFORM_OFFSET, SCALE_STEP * C_Y, hiLight);
}

void DSODisplay::drawVerticalTrigger(bool draw, int column)
{
    if (draw)
    {
        tft->VLine(column, DSO_WAVEFORM_OFFSET + 1, DSO_WAVEFORM_HEIGHT - 1, ILI_RED);
    }
    else
    {
        bool repeat;
        const uint16_t *bg = getHzBackGround(column, repeat);
        tft->setAddress(column, 1 + DSO_WAVEFORM_OFFSET, 1, DSO_WAVEFORM_HEIGHT - 1);
        if (repeat)
            tft->floodWords(DSO_WAVEFORM_HEIGHT, bg[0]);
        else
            tft->pushColors(DSO_WAVEFORM_HEIGHT, (uint16_t *)bg);
    }
}

void DSODisplay::drawVoltageTrigger(bool draw, int line)
{
    AutoGfx autogfx;

    line = DSO_WAVEFORM_HEIGHT / 2 + DSO_WAVEFORM_OFFSET - line;
    line = XMAX(line, 1);
    line = XMIN(line, DSO_WAVEFORM_HEIGHT - 1);

    if (draw)
    {
        tft->HLine(1, 1 + line, DSO_WAVEFORM_WIDTH - 2, ILI_BLUE);
    }
    else
    {
        bool repeat;
        const uint16_t *bg = getHzBackGround(line, repeat);
        tft->setAddress(0, 1 + line, DSO_WAVEFORM_WIDTH, 1);
        if (repeat)
            tft->floodWords(DSO_WAVEFORM_WIDTH, bg[0]);
        else
            tft->pushColors(DSO_WAVEFORM_WIDTH, (uint16_t *)bg);
    }
}

void prettyPrint(float x, int maxW)
{
    float a = fabs(x);
    if (a < 0.8f)
    {
        if (a == 0.0f)
            sprintf(textBuffer, "0");
        else if (a < 0.001f)
            sprintf(textBuffer, "%03du", (int)(1000000.f * x));
        else
            sprintf(textBuffer, "%03dm", (int)(1000.f * x));
    }
    else
    {
        format_float_02_2f(x, 2);
    }

    if (maxW)
        tft->printUpTo(textBuffer, maxW);
    else
        tft->print(textBuffer);
}

static __attribute__((noinline)) void drawInfoHeader(int line, const char *info, int color)
{
    TITLE_SQUARE(line, color);
    tft->print(DSO_INFO_START_COLUMN + 2, DSO_HEIGHT_OFFSET + HeaderLine[line] * DSO_CHAR_HEIGHT - LINE_OFFSET, info);
}

void DSODisplay::printTriggerValue(float volt, bool hilight)
{
    AutoGfx autogfx;
    tft->setTextColor(hilight ? ILI_BLACK : ILI_WHITE, hilight ? ILI_WHITE : ILI_BLACK);
    printMeasurement(TRIGGER_ROW, volt);
}

void DSODisplay::printOffsetValue(float volt, bool hilight)
{
    AutoGfx autogfx;
    tft->setTextColor(hilight ? ILI_BLACK : ILI_WHITE, hilight ? ILI_WHITE : ILI_BLACK);
    printMeasurement(VOLTAGE_OFFSET_ROW, volt);
}

struct HeaderDesc
{
    int16_t row;
    const char *data;
};
static const HeaderDesc Headers[] = {
    {MIN_ROW, "MnMx"}, {FREQ_ROW, "Freq"}, {TRIGGER_ROW, "Trigg"}, {VOLTAGE_OFFSET_ROW, "Offset"}, {-1, ""},
};

void DSODisplay::drawStatsBackGround()
{
    AutoGfx autogfx;
    tft->VLine(DSO_INFO_START_COLUMN, 0, BG_STATS_HEIGHT, BG_COLOR);
    tft->VLine(319, 0, BG_STATS_HEIGHT, BG_COLOR);
    tft->setTextColor(ILI_BLACK, BG_COLOR);
    for (const HeaderDesc *h = Headers; h->row != -1; h++)
        drawInfoHeader(h->row, h->data, BG_COLOR);
    tft->setTextColor(BG_COLOR, ILI_BLACK);
    oldMode = DSO_CAPTURE_MODE_INVALIDE;
}

void lowBarPrint(int column, const char *st)
{
    AutoGfx autogfx;
    tft->setCursor((column - 1) * DSO_LOW_BAR_BUTTON + 1, 240 - 2);
    tft->square(0, (column - 1) * DSO_LOW_BAR_BUTTON + 1, 240 - 20, DSO_LOW_BAR_BUTTON, 20);
    tft->print(st);
}

static void genericDraw(int column, const char *v, bool highlight)
{
    int fg = highlight ? ILI_BLACK : LIGHT_ILI_GREEN;
    int button = highlight ? ILI_BLUE : ILI_BLACK;
    int bg = ILI_BLACK;

    tft->setTextColor(fg, highlight ? LIGHT_ILI_GREEN : ILI_BLACK);
    lowBarPrint(column, v);
    {
        AutoGfx autogfx;
        tft->centeredButton((column - 1) * DSO_LOW_BAR_BUTTON + 1, 240 - 20, DSO_LOW_BAR_BUTTON, 20, v, fg, button, bg);
        tft->setTextColor(LIGHT_ILI_GREEN, ILI_BLACK);
    }
}

void DSODisplay::drawVolt(const char *v, bool highlight)
{
    genericDraw(VOLTAGE_MODE, v, highlight);
}
void DSODisplay::drawTrigger(const char *v, bool highlight)
{
    genericDraw(TRIGGER_MODE, v, highlight);
}
void DSODisplay::drawTime(const char *v, bool highlight)
{
    genericDraw(TIME_MODE, v, highlight);
}
void DSODisplay::drawCoupling(const char *v, bool highlight)
{
    genericDraw(STAT_MODE, v, highlight);
}
void DSODisplay::drawArming(const char *v, bool highlight)
{
    genericDraw(ARMING_MODE, v, highlight);
}

DSODisplay::MODE_TYPE DSODisplay::getMode()
{
    return mode;
}

void DSODisplay::setMode(MODE_TYPE t)
{
    if ((mode & 0x80) ^ !(t & 0x80)) // leaving or entering alt
        drawStatsBackGround();
    mode = t;
}

void DSODisplay::drawAutoSetup(void)
{
    AutoGfx autogfx;
    tft->fillRoundRect(320 / 2 - AUTOCAL_BOX_WIDTH / 2, 240 / 2 - AUTOCAL_BOX_HEIGHT / 2, AUTOCAL_BOX_WIDTH,
                       AUTOCAL_BOX_HEIGHT, 4, AUTOCAL_COLOR, 0);
}

void DSODisplay::drawAutoSetupStep(int i)
{
    AutoGfx autogfx;
    int pg = ((AUTOCAL_BOX_WIDTH - AUTOCAL_BORDER * 2) * i) / 3;
    tft->fillRoundRect(320 / 2 - AUTOCAL_BOX_WIDTH / 2 + AUTOCAL_BORDER,
                       240 / 2 - AUTOCAL_BOX_HEIGHT / 2 + AUTOCAL_BORDER, pg, AUTOCAL_BOX_HEIGHT - AUTOCAL_BORDER * 2,
                       4, AUTOCAL_COLOR, AUTOCAL_COLOR);
}

void DSODisplay::printString(int x, int y, const char *text)
{
    AutoGfx autogfx;
    tft->setFontSize(ili9341::MediumFont);
    tft->setTextColor(ILI_WHITE, ILI_BLACK);
    tft->setCursor(x, y);
    tft->print(text);
    tft->setFontSize(ili9341::SmallFont);
}

void DSODisplay::clearScreen()
{
    AutoGfx autogfx;
    tft->fillScreen(ILI_BLACK);
}

void DSODisplay::drawSplash()
{
    AutoGfx autogfx;
    tft->fillScreen(ILI_BLACK);
    tft->drawHSBitmap(splash_width, splash_height, 2, 2, ILI_WHITE, ILI_BLACK, getSplash());
    tft->setFontSize(ili9341::SmallFont);
    tft->setTextColor(ILI_WHITE, ILI_BLACK);

    tft->setCursor(140, 64);
    tft->print("lnDSO150");
    tft->setCursor(140, 84);
#ifdef USE_RXTX_PIN_FOR_ROTARY
    tft->print("USB  Version");
#else
    tft->print("RXTX Version");
#endif

    char bf[20];
    sprintf(bf, "%d.%02d", DSO_VERSION_MAJOR, DSO_VERSION_MINOR);
    tft->setCursor(140, 64 + 20 * 2);
    tft->print(bf);

    tft->setCursor(140, 64 + 20 * 4);
    tft->print(lnCpuID::idAsString());

    tft->setCursor(140, 64 + 20 * 5);
    sprintf(bf, "%d Mhz", lnCpuID::clockSpeed() / 1000000);
    tft->print(bf);

    xDelay(100);
}

void DSODisplay::drawTriggered(bool triggered)
{
    AutoGfx autogfx;
    int color = triggered ? ILI_GREEN : ILI_RED;
    tft->square(color, DSO_INFO_START_COLUMN + LED_X_OFFSET, LED_Y_OFFSET, LED_WIDTH, LED_HEIGHT);
}
// EOF
