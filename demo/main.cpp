#include "esprit.h"
//
#include "raylib.h"
//
#include "dso_display.h"
#include "ili_ray.h"
rayIli *ray = NULL;
int main()
{

    const int screenWidth = 320;
    const int screenHeight = 240;

    InitWindow(screenWidth * 2, screenHeight * 2, "DSO ");

    ray = new rayIli(screenWidth, screenHeight);
    DSODisplay::init(ray);
    while (!WindowShouldClose())
    {
        DSODisplay::drawGrid();
        ray->sync();
        xDelay(1000);
    }
    CloseWindow();
    return 0;
}
