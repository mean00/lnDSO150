#pragma once

/**
 */
class MenuItem
{
public:
  typedef enum
  {
    MENU_TITLE, // should be the 1st one
    MENU_KEY_VALUE,
    MENU_SUBMENU,
    MENU_CALL,
    MENU_END,
    MENU_TOGGLE,
    MENU_BACK
  }MenuItemType;
  const MenuItemType type;
  const char *menuText;  
  const void *cookie;
};
/**
 * 
 * @param menu
 */
class MenuManager
{
public:
    MenuManager(const MenuItem *menu);
    ~MenuManager();
    void run(void);
protected:
    const MenuItem *_menu;
          void runOne(const MenuItem *xtop);    
          void printMenuEntry(bool onoff, int line,const char *text);
          void printMenuTitle(const char *text);
          void redraw(const char *title, int n,const MenuItem *xtop, int current);
          void blink(int current, const char *text);
    
};