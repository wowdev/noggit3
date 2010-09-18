#ifndef __TREEVIEW_H
#define __TREEVIEW_H

class TreeView;
class TreeViewButton;

#include "noggit.h"
#include "video.h"
#include "textUI.h"
#include "buttonUI.h"
#include "directory.h"

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

	void Expand( );
	void Minimize( );
	bool Expanded( );
	void Toggle( );

	void SetSelectFunction( void (*pSelectFunction)( const std::string& ) );
	std::string GetDirectoryName( )
	{
		return mMyDir->mName;
	}
	TreeView * GetParent( )
	{
		return mParent;
	}

	void Move( int pEntries, TreeView * pFrom );

	void render();
	
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
