#include "UITreeView.h"

#include <algorithm>
#include <map>
#include <string>
#include <vector>

#include "Directory.h"
#include "Noggit.h" // arial12
#include "UIText.h"
#include "Video.h"

UITreeViewButton::UITreeViewButton( float _x, float _y, UITreeView* pTreeView )
: UIButton(  _x, _y, 12.0f, 12.0f, "Interface\\Buttons\\UI-PlusButton-Up.blp", "Interface\\Buttons\\UI-PlusButton-Down.blp" )
, mTreeView( pTreeView )
{
}

UIFrame * UITreeViewButton::processLeftClick( float /*mx*/, float /*my*/ )
{
  SetClicked( true );
  mTreeView->Toggle();
  return this;
}

void UITreeViewButton::SetClicked( bool pClicked )
{
  clicked = pClicked;
}

UITreeView::UITreeView( float pX, float pY, Directory * pDirectory, UITreeView * pParent, void (*pSelectFunction)( const std::string& ) )
: UIFrame( pX, pY, 0.0f, 0.0f )
, mParent( pParent )
, mMyDir( pDirectory )
, mMyButton( new UITreeViewButton( 0, 0, this ) )
, mMyText( new UIText( 13, 0, mMyDir->mName, arial12, eJustifyLeft ) )
, mSelectFunction( pSelectFunction )
, mExpanded( false )
{
  float lY = 13.0f;

  std::map<std::string,Directory*>::iterator it;
  for( it = mMyDir->mSubdirectories.begin(); it != mMyDir->mSubdirectories.end(); ++it )
  {
    mOthers.push_back( new UITreeView( 13.0f, lY, it->second, this, pSelectFunction ) );
    lY = lY + 13.0f;
  }
  std::vector<File*>::iterator itfile;
  for( itfile = mMyDir->mSubfiles.begin(); itfile != mMyDir->mSubfiles.end(); ++itfile )
  {
    UIText * temp = new UIText( 13.0f, lY, (*itfile)->mName, arial12, eJustifyLeft );
    mFiles.push_back( temp );
    lY = lY + 13.0f;
  }
}

void UITreeView::Expand()
{
  mExpanded = true;
}

void UITreeView::Minimize()
{
  mExpanded = false;
}

bool UITreeView::Expanded()
{
  return mExpanded;
}

std::string UITreeView::GetDirectoryName()
{
  return mMyDir->mName;
}

void UITreeView::Move( int pEntries, UITreeView * pFrom )
{
  std::vector<UITreeView*>::iterator trees = std::find( mOthers.begin(), mOthers.end(), pFrom );
  trees++;
  while( trees != mOthers.end() )
  {
    (*trees)->y = (*trees)->y + pEntries * 13;
    trees++;
  }

  std::vector<UIText*>::iterator childfiles;
  for( childfiles = mFiles.begin(); childfiles != mFiles.end(); ++childfiles )
  {
    (*childfiles)->y = (*childfiles)->y + pEntries * 13;
  }

  if( mParent )
  {
    mParent->Move( pEntries, this );
  }
}

void UITreeView::Toggle()
{
  mExpanded = !mExpanded;
  mMyButton->SetClicked( mExpanded );

  std::vector<UITreeView*>::iterator childtreeviews;
  for( childtreeviews = mOthers.begin(); childtreeviews != mOthers.end(); ++childtreeviews )
  {
    if( (*childtreeviews)->Expanded() )
      (*childtreeviews)->Toggle();
  }
  
  if( mParent )
    if( !mExpanded )
      mParent->Move( -static_cast<int>( ( mOthers.size() + mFiles.size() ) ), this );
    else
      mParent->Move( ( mOthers.size() + mFiles.size() ), this );
}

void UITreeView::render() const
{
  if( hidden )
    return;

  glPushMatrix();
  glTranslatef( x, y, 0 );
  mMyButton->render();
  mMyText->render();

  if( mExpanded )
  {
    std::vector<UITreeView*>::const_iterator childtreeviews;
    for( childtreeviews = mOthers.begin(); childtreeviews != mOthers.end(); ++childtreeviews )
    {
      (*childtreeviews)->render();
    }
    std::vector<UIText*>::const_iterator childfiles;
    for( childfiles = mFiles.begin(); childfiles != mFiles.end(); ++childfiles )
    {
      (*childfiles)->render();
    }
  }

  glPopMatrix();
}

void UITreeView::SetSelectFunction( void (*pSelectFunction)( const std::string& ) )
{
  mSelectFunction = pSelectFunction;
}

UIFrame * UITreeView::processLeftClick( float mx, float my )
{
  if( hidden )
    return 0;

  mx -= x;
  my -= y;
  if((!mMyButton->hidden)&&(mMyButton->x<mx)&&(mMyButton->x+mMyButton->width>mx)&&(mMyButton->y<my)&&(mMyButton->y+mMyButton->height>my))
    return mMyButton->processLeftClick(mx-mMyButton->x,my-mMyButton->y);

  std::vector<UIText*>::iterator childfiles;
  if( mSelectFunction )
    for( childfiles = mFiles.begin(); childfiles != mFiles.end(); ++childfiles )
    {
      if( (!(*childfiles)->hidden)&&((*childfiles)->x<mx)&&((*childfiles)->x+(*childfiles)->width>mx)&&((*childfiles)->y<my)&&((*childfiles)->y+(*childfiles)->height>my) )
      {
        std::string lPath;
        UITreeView * lParent = this;
        while( lParent )
        {
          lPath.insert( 0, std::string( lParent->GetDirectoryName() + "/" ) );
          lParent = lParent->GetParent();
        }
        mSelectFunction( lPath + (*childfiles)->getText() );
        return *childfiles;
      }
    }

  if( mExpanded )
  {
    std::vector<UITreeView*>::iterator childtreeviews;
    for( childtreeviews = mOthers.begin(); childtreeviews != mOthers.end(); ++childtreeviews )
    {
      (*childtreeviews)->processLeftClick(mx,my);
    }
  }

  return 0;
}
