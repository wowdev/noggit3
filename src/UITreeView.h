#ifndef __TREEVIEW_H
#define __TREEVIEW_H

#include <string>
#include <vector>

#include "UIButton.h"
#include "UIFrame.h"

class UIText;
struct Directory;

class UITreeViewButton;

class UITreeView : public UIFrame
{
  UITreeView * mParent;
public:
  std::vector<UITreeView*> mOthers;
private:
  std::vector<UIText*> mFiles;
  Directory * mMyDir;
  UITreeViewButton * mMyButton;
  UIText * mMyText;

  void (*mSelectFunction)( const std::string& );  

  bool mExpanded;

public:
  UITreeView( float pX, float pY, Directory * pDirectory, UITreeView * pParent, void (*pSelectFunction)( const std::string& ) );

  void Expand();
  void Minimize();
  bool Expanded();
  void Toggle();

  void SetSelectFunction( void (*pSelectFunction)( const std::string& ) );
  std::string GetDirectoryName();
  UITreeView * GetParent()
  {
    return mParent;
  }

  void Move( int pEntries, UITreeView * pFrom );

  void render() const;
  
  UIFrame * processLeftClick( float mx,float my );
};

class UITreeViewButton : public UIButton
{
private:
  UITreeView* mTreeView;
public:
  UITreeViewButton( float x, float y, UITreeView* pTreeView );

  UIFrame* processLeftClick(float mx,float my);

  void SetClicked( bool pClicked );
};
#endif
