#ifndef __MENUBAR_H
#define __MENUBAR_H

class MenuButton;
class MenuPane;
class MenuItem;
class MenuItemButton;
class MenuItemToggle;
class MenuItemSeperator;
class menuBar;

#include "Gui.h"
#include "buttonUI.h"
#include "checkboxUI.h"

// This file contains more than one class as they are needed together.

class MenuButton : public buttonUI
{
private:
	MenuPane * mPane;
public:
	MenuButton( MenuPane * pPane, float pX, float pY, std::string pText );

	frame* processLeftClick( float pX, float pY );
};

class MenuItem : public buttonUI 
{
protected:
	MenuPane * mParent;

public:
	MenuItem( MenuPane * pParent, float pX, float pY, float pHeight, std::string pText, std::string pNormal, std::string pDown );
};

class MenuItemButton : public MenuItem 
{
public:
	MenuItemButton( MenuPane * pParent, float pX, float pY, std::string pText, void ( *pClickFunc )( frame *, int ), int pClickFuncID );

	frame* processLeftClick( float pX, float pY );
};

class MenuItemToggle : public MenuItem 
{
private:
	checkboxUI * mMyCheckbox;
	bool * mMyState;
	bool mInvert;

public:
	MenuItemToggle( MenuPane * pParent, float pX, float pY, std::string pText, bool * pMyState, bool pInvert = false );

	frame* processLeftClick( float pX, float pY );

	void render( );
};

class MenuItemSwitch : public MenuItem 
{
private:
	bool * mMyState;
	bool mInvert;

public:
	MenuItemSwitch( MenuPane * pParent, float pX, float pY, std::string pText, bool * pMyState, bool pInvert = 1 );

	frame* processLeftClick( float pX, float pY );

	void render( );
};


class MenuItemSet : public MenuItem 
{
private:
	int mSet;
	int * mMyState;

public:
	MenuItemSet( MenuPane * pParent, float pX, float pY, std::string pText, int * pMyState, int pSet = 1 );

	frame* processLeftClick( float pX, float pY );

	void render( );
};

class MenuItemSeperator : public MenuItem
{
public:
	MenuItemSeperator( MenuPane * pParent, float pX, float pY, std::string pText );

	frame* processLeftClick( float pX, float pY );
};

class MenuPane : public window
{
private:
	int mNumItems;
	menuBar * mMenuBar;

public: 
	MenuPane( menuBar * pMenuBar, float pX, float pY );

	void Close();
	void Open();
	
	void AddMenuItemButton( std::string pName, void ( *pClickFunc )( frame *, int ), int pClickFuncID );
	void AddMenuItemToggle( std::string pName, bool * pMyState, bool pInvert = false );
	void AddMenuItemSwitch( std::string pName, bool * pMyState, bool pInvert = false );
	void AddMenuItemSet( std::string pName, int * pMyIntState, int pSet = 1 );

	void AddMenuItemSeperator( std::string pName );
};


class menuBar : public window
{
private:
	Gui *mainGui;

	std::map<std::string,MenuPane*> mMenuPanes;

	int mNumMenus;

public:
	bool mustResize;

	menuBar( );
	void render();	
	void resize();

	void CloseAll( );
	void ClearAll( );
	void AddMenu( std::string pName );

	MenuPane * GetMenu( std::string pName );

	frame * processLeftClick(float mx,float my);
};
#endif
