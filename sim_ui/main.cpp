#include "esprit.h"
//
#include "raylib.h"
//
#include "dso_control.h"
#include "dso_display.h"
#include "dso_gfx.h"
//
#include "ili_ray.h"

rayIli *ray = NULL;
const GFXfont *smallFont();
const GFXfont *mediumFont();
const GFXfont *bigFont();
//
void redrawEverything();
//
void menuManagement(DSOControl *control);
//
#undef printf

void control_cb(DSOControl::DSOEvent evt)
{
    printf("DSO Event : %d\n", (int)evt);
}

int main()
{

    const int screenWidth = 320;
    const int screenHeight = 240;

    InitWindow(screenWidth * 2, screenHeight * 2, "DSO ");

    DSOControl control(control_cb);

    ray = new rayIli(screenWidth, screenHeight);
    ray->setFontFamily(smallFont(), mediumFont(), bigFont());
    ray->setFontSize(ili9341::SmallFont);
    ray->setTextColor(ILI_GREEN, ILI_BLACK);
    DSO_GFX::init(ray);
    DSODisplay::init(ray);
    //
    //
    DSODisplay::drawTrigger("A   A", false);
    redrawEverything();
    DSODisplay::printTriggerValue(-0.03, false);
    DSODisplay::printOffsetValue(23., true);
    DSODisplay::drawVolt("500mV", false);
    DSODisplay::drawTrigger("Up", false);
    DSODisplay::drawTime("20us", false);
    DSODisplay::drawArming("Single", false);
    DSODisplay::drawCoupling("AC", false);
    // individual item on the screen
    float imin = -0.3;
    float imax = 45.5;
    uint32_t fq = 200;
#if 1
    menuManagement(&control);
#else
    while (!WindowShouldClose())
    {
        DSODisplay::drawFq(fq);
        fq += 333;
        // printf("Fq=%d\n", fq);
        DSODisplay::drawMinMax(imin, imax);
        imin -= 0.01;
        imax += 0.01;
        DSODisplay::drawGrid();
        DSODisplay::drawStatsBackGround();
        DSODisplay::drawVolt("1.3", true);
        ray->sync();
        xDelay(50);
    }
    CloseWindow();
#endif
    return 0;
}

extern "C" void _putchar(char foo)
{
    printf("%c", foo);
}
/**
 *
 */
void sim_redraw()
{
    ray->sync();
}

/**
 *
 */
void redrawEverything()
{
    DSODisplay::cleanup();
    // DSO_GFX::setSmallFont();
    DSODisplay::drawGrid();
    DSODisplay::drawStatsBackGround();
    DSODisplay::drawCoupling("Gnd", false);
    DSODisplay::drawTrigger("_-", false);
}

/**
 *
 */
