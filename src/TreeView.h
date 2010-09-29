#ifndef __TREEVIEW_H
#define __TREEVIEW_H

#include "frame.h"
#include "buttonUI.h"

class textUI;
class Directory;

class TreeViewButton;

class TreeView : public frame
{
	TreeView * mParent;
public:
	std::vector<TreeView*> mOthers;
private:
	std::vector<textUI*> mFiles;
	Directory * mMyDir;
	TreeViewButton * mMyButton;
	textUI * mMyText;

	void (*mSelectFunction)( const std::string& );	

	bool mExpanded;

public:
	TreeView( float pX, float pY, Directory * pDirectory, TreeView * pParent, void (*pSelectFunction)( const std::string& ) );

	void Expand();
	void Minimize();
	bool Expanded();
	void Toggle();

	void SetSelectFunction( void (*pSelectFunction)( const std::string& ) );
	std::string GetDirectoryName();
	TreeView * GetParent()
	{
		return mParent;
	}

	void Move( int pEntries, TreeView * pFrom );

	void render() const;
	
	frame * processLeftClick( float mx,float my );
};

class TreeViewButton : public buttonUI
{
private:
	TreeView * mTreeView;
public:
	TreeViewButton( float x, float y, TreeView* pTreeView );

	frame * processLeftClick(float mx,float my);

	void SetClicked( bool pClicked );
};
#endif
