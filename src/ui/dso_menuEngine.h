#pragma once
#include "lnArduino.h"
/**
 */
class MenuItem
{
public:
  typedef enum
  {
    MENU_TITLE, // should be the 1st one    
    MENU_SUBMENU,
    MENU_CALL,
    MENU_END,
    MENU_TOGGLE,
    MENU_INDEX,
    MENU_TEXT,
    MENU_BACK
  }MenuItemType;
  const MenuItemType type;
  const char *menuText;  
  const void *cookie;
};

struct MenuListItem
{
public:
    int               *item;
    const   int       thisItem;
};

/**
 * 
 * @param menu
 */
class DSOControl;
class MenuManager
{
public:
    MenuManager(DSOControl *ctl, const MenuItem *menu);
    ~MenuManager();
    void run(void);
    
public:
          void controlEvent();
protected:
    const MenuItem *_menu;
          void runOne(const MenuItem *xtop);    
          void printMenuEntry(bool onoff, int line,const char *text);
          void printMenuTitle(const char *text);
          void printPrefix(bool onoff, int line,const char *text);
          void redraw(const char *title, int n,const MenuItem *xtop, int current);
          void blink(int current, const char *text);
          void printBackHint();
       
protected:
          bool              handlePress(const char *title,int n,const MenuItem *xtop,int current);
          void              runOne_( const MenuItem *xtop);
          xBinarySemaphore  _sem;
          DSOControl        *_control;    
};