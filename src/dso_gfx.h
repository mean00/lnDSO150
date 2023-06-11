
#include "dso_colors.h"
#pragma once
class ili9341;

/**
 *
 * @param _ili
 */
class DSO_GFX
{
  public:
    static void init(ili9341 *_ili);
    static void newPage(const char *title);
    static void subtitle(const char *title);
    static void bottomLine(const char *title);
    static void printxy(int x, int y, const char *t);
    static void printButton(int x, int y, int length, const char *t, int fg, int button, int bg);
    static void clear(int color);
    static void setTextColor(int fg, int bg);
    static void setBigFont(bool big);
    static void setSmallFont();
    static void printMenuTitle(const char *text);
    static void clearBody(int color);
    static void center(const char *text, int line);

  protected:
    static void markup(const char *t);
};