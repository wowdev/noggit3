#ifndef __MENUBAR_H
#define __MENUBAR_H

#include <map>
#include <string>

#include "UIButton.h"
#include "UIWindow.h"
#include "UICheckBox.h"

// This file contains more than one class as they are needed together.

class MenuPane : public UIWindow
{
public:
  typedef MenuPane* Ptr;

private:
  int mNumItems;

public:
  MenuPane( float pX, float pY );

  void Close();
  void Open();

  void fixSizes();

  void AddMenuItemButton( const std::string& pName, void ( *pClickFunc )( UIFrame::Ptr, int ), int pClickFuncID );
  void AddMenuItemToggle( const std::string& pName, bool * pMyState, bool pInvert = false );
  void AddMenuItemSwitch( const std::string& pName, bool * pMyState, bool pInvert = false );
  void AddMenuItemSet( const std::string& pName, int * pMyIntState, int pSet = 1 );

  void AddMenuItemSeperator( const std::string& pName );
};

class UIMenuBar : public UIWindow
{
public:
  typedef UIMenuBar* Ptr;

private:
  typedef std::map<std::string, MenuPane::Ptr> MenuPanes;
  MenuPanes mMenuPanes;

  int mNumMenus;

public:
  UIMenuBar();
  void render() const;
  void resize();

  void CloseAll();
  void ClearAll();
  void AddMenu( const std::string& pName );

  MenuPane::Ptr GetMenu( const std::string& pName );

  UIFrame::Ptr processLeftClick( float mx, float my );
};

class MenuButton : public UIButton
{
public:
  typedef MenuButton* Ptr;

private:
  MenuPane::Ptr mPane;

public:
  MenuButton( MenuPane::Ptr pPane, float pX, float pY, const std::string& pText );

  UIFrame::Ptr processLeftClick( float pX, float pY );
};

class MenuItem : public UIButton
{
public:
  typedef MenuItem* Ptr;

protected:
  MenuPane::Ptr mParent;

public:
  MenuItem( MenuPane::Ptr pParent, float pX, float pY, float pHeight, const std::string& pText, const std::string& pNormal, const std::string& pDown );
};

class MenuItemButton : public MenuItem
{
public:
  typedef MenuItemButton* Ptr;

  MenuItemButton( MenuPane::Ptr pParent, float pX, float pY, const std::string& pText, void ( *pClickFunc )( UIFrame::Ptr, int ), int pClickFuncID );

  UIFrame::Ptr processLeftClick( float pX, float pY );
};

class MenuItemToggle : public MenuItem
{
public:
  typedef MenuItemToggle* Ptr;

private:
  UICheckBox::Ptr mMyCheckbox;
  bool * mMyState;
  bool mInvert;

public:
  MenuItemToggle( MenuPane::Ptr pParent, float pX, float pY, const std::string& pText, bool * pMyState, bool pInvert = false );

  UIFrame::Ptr processLeftClick( float pX, float pY );

  void render() const;
};

class MenuItemSwitch : public MenuItem
{
public:
  typedef MenuItemSwitch* Ptr;

private:
  bool * mMyState;
  bool mInvert;

public:
  MenuItemSwitch( MenuPane::Ptr pParent, float pX, float pY, const std::string& pText, bool * pMyState, bool pInvert = 1 );

  UIFrame::Ptr processLeftClick( float pX, float pY );
};


class MenuItemSet : public MenuItem
{
public:
  typedef MenuItemSet* Ptr;

private:
  int mSet;
  int * mMyState;

public:
  MenuItemSet( MenuPane::Ptr pParent, float pX, float pY, const std::string& pText, int * pMyState, int pSet = 1 );

  UIFrame::Ptr processLeftClick( float pX, float pY );
};

class MenuItemSeperator : public MenuItem
{
public:
  typedef MenuItemSeperator* Ptr;

  MenuItemSeperator( MenuPane::Ptr pParent, float pX, float pY, const std::string& pText );

  UIFrame::Ptr processLeftClick( float pX, float pY );
};

#endif
