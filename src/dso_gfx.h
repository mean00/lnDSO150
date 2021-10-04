
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
  static void markup(const char *title);
  static void center(const char *p,int y);
  static void printxy(int x, int y, const char *t);
  static void clear(int color);
  static void setTextColor(int fg,int bg);  
};