#ifndef __MENUBAR_H
#define __MENUBAR_H

#include <map>
#include <string>

#include "UIButton.h"
#include "UIWindow.h"

class MenuButton;
class MenuPane;
class MenuItem;
class MenuItemButton;
class MenuItemToggle;
class MenuItemSeperator;
class UIMenuBar;

class UICheckBox;

// This file contains more than one class as they are needed together.

class MenuButton : public UIButton
{
private:
  MenuPane * mPane;
public:
  MenuButton( MenuPane * pPane, float pX, float pY, const std::string& pText );

  UIFrame* processLeftClick( float pX, float pY );
};

class MenuItem : public UIButton 
{
protected:
  MenuPane * mParent;

public:
  MenuItem( MenuPane * pParent, float pX, float pY, float pHeight, const std::string& pText, const std::string& pNormal, const std::string& pDown );
};

class MenuItemButton : public MenuItem 
{
public:
  MenuItemButton( MenuPane * pParent, float pX, float pY, const std::string& pText, void ( *pClickFunc )( UIFrame *, int ), int pClickFuncID );

  UIFrame* processLeftClick( float pX, float pY );
};

class MenuItemToggle : public MenuItem 
{
private:
  UICheckBox * mMyCheckbox;
  bool * mMyState;
  bool mInvert;

public:
  MenuItemToggle( MenuPane * pParent, float pX, float pY, const std::string& pText, bool * pMyState, bool pInvert = false );

  UIFrame* processLeftClick( float pX, float pY );

  void render() const;
};

class MenuItemSwitch : public MenuItem 
{
private:
  bool * mMyState;
  bool mInvert;

public:
  MenuItemSwitch( MenuPane * pParent, float pX, float pY, const std::string& pText, bool * pMyState, bool pInvert = 1 );

  UIFrame* processLeftClick( float pX, float pY );
};


class MenuItemSet : public MenuItem 
{
private:
  int mSet;
  int * mMyState;

public:
  MenuItemSet( MenuPane * pParent, float pX, float pY, const std::string& pText, int * pMyState, int pSet = 1 );

  UIFrame* processLeftClick( float pX, float pY );
};

class MenuItemSeperator : public MenuItem
{
public:
  MenuItemSeperator( MenuPane * pParent, float pX, float pY, const std::string& pText );

  UIFrame* processLeftClick( float pX, float pY );
};

class MenuPane : public UIWindow
{
private:
  int mNumItems;
  UIMenuBar * mMenuBar;

public: 
  MenuPane( UIMenuBar * pMenuBar, float pX, float pY );

  void Close();
  void Open();
  
  void fixSizes();
  
  void AddMenuItemButton( const std::string& pName, void ( *pClickFunc )( UIFrame *, int ), int pClickFuncID );
  void AddMenuItemToggle( const std::string& pName, bool * pMyState, bool pInvert = false );
  void AddMenuItemSwitch( const std::string& pName, bool * pMyState, bool pInvert = false );
  void AddMenuItemSet( const std::string& pName, int * pMyIntState, int pSet = 1 );

  void AddMenuItemSeperator( const std::string& pName );
};


class UIMenuBar : public UIWindow
{
private:
  std::map<std::string,MenuPane*> mMenuPanes;

  int mNumMenus;

public:
  UIMenuBar();
  void render() const;  
  void resize();

  void CloseAll();
  void ClearAll();
  void AddMenu( const std::string& pName );

  MenuPane* GetMenu( const std::string& pName );

  UIFrame* processLeftClick( float mx, float my );
};
#endif
