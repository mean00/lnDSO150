#include "esprit.h"
//
#include "raylib.h"
//
#include "dso_display.h"
#include "ili_ray.h"
rayIli *ray = NULL;
const GFXfont *smallFont();
const GFXfont *mediumFont();
const GFXfont *bigFont();
int main()
{

    const int screenWidth = 320;
    const int screenHeight = 240;

    InitWindow(screenWidth * 2, screenHeight * 2, "DSO ");

    ray = new rayIli(screenWidth, screenHeight);
    ray->setFontFamily(smallFont(), mediumFont(), bigFont());
    ray->setFontSize(ili9341::SmallFont);
    ray->setTextColor(ILI_GREEN, ILI_BLACK);
    DSODisplay::init(ray);
    while (!WindowShouldClose())
    {
        DSODisplay::drawGrid();
        DSODisplay::drawStatsBackGround();
        DSODisplay::drawVolt("1.3", true);
        ray->sync();
        xDelay(1000);
    }
    CloseWindow();
    return 0;
}
