#pragma once

/**
 */
class MenuItem
{
public:
  typedef enum
  {
    MENU_KEY_VALUE,
    MENU_SUBMENU,
    MENU_CALL,
    MENU_END,
    MENU_TOGGLE
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
    MenuManager(const char *title,const MenuItem *menu);
    ~MenuManager();
    void run(void);
protected:
    const MenuItem *_menu;
    const char *_title;
    
    void printMenuEntry(bool onoff, int line,const char *text);
    
};