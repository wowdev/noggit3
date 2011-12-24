#include <noggit/UITreeView.h>

#include <algorithm>
#include <map>
#include <string>
#include <vector>

#include <noggit/Directory.h>
#include <noggit/Noggit.h> // arial12
#include <noggit/UIText.h>
#include <noggit/Video.h>

UITreeViewButton::UITreeViewButton( float _x, float _y, UITreeView::Ptr pTreeView )
: UIButton(  _x, _y, 12.0f, 12.0f, "Interface\\Buttons\\UI-PlusButton-Up.blp", "Interface\\Buttons\\UI-PlusButton-Down.blp" )
, mTreeView( pTreeView )
{
}

UIFrame::Ptr UITreeViewButton::processLeftClick( float /*mx*/, float /*my*/ )
{
  SetClicked( true );
  mTreeView->Toggle();
  return this;
}

void UITreeViewButton::SetClicked( bool pClicked )
{
  clicked = pClicked;
}

UITreeView::UITreeView( float pX, float pY, const std::string& directoryName, Directory::Ptr pDirectory, UITreeView::Ptr pParent, void (*pSelectFunction)( const std::string& ) )
: UIFrame( pX, pY, 0.0f, 0.0f )
, mParent( pParent )
, mMyDir( pDirectory )
, mMyButton( new UITreeViewButton( 0, 0, UITreeView::Ptr( this ) ) )
, mMyText( new UIText( 13, 0, directoryName, arial12, eJustifyLeft ) )
, _directoryName( directoryName )
, mSelectFunction( pSelectFunction )
, mExpanded( false )
{
  float lY = 13.0f;

  for( Directory::Directories::const_iterator it( mMyDir->directoriesBegin() ), end( mMyDir->directoriesEnd() )
     ; it != end
     ; ++it
     )
  {
    _others.push_back( UITreeView::Ptr( new UITreeView( 13.0f, lY, it->first, it->second, UITreeView::Ptr( this ), pSelectFunction ) ) );
    lY = lY + 13.0f;
  }
  for( Directory::Files::const_iterator it( mMyDir->filesBegin() ), end( mMyDir->filesEnd() )
     ; it != end
     ; ++it
     )
  {
    mFiles.push_back( new UIText( 13.0f, lY, *it, arial12, eJustifyLeft ) );
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

const std::string& UITreeView::GetDirectoryName()
{
  return _directoryName;
}

void UITreeView::Move( int pEntries, UITreeView::Ptr pFrom )
{
  Others::iterator trees = std::find( _others.begin(), _others.end(), pFrom );
  ++trees;
  while( trees != _others.end() )
  {
    (*trees)->y( (*trees)->y() + pEntries * 13.0f );
    ++trees;
  }

  std::vector<UIText*>::iterator childfiles;
  for( childfiles = mFiles.begin(); childfiles != mFiles.end(); ++childfiles )
  {
    (*childfiles)->y( (*childfiles)->y() + pEntries * 13.0f );
  }

  if( mParent )
  {
    mParent->Move( pEntries, shared_from_this() );
  }
}

void UITreeView::Toggle()
{
  mExpanded = !mExpanded;
  mMyButton->SetClicked( mExpanded );

  Others::iterator childtreeviews;
  for( childtreeviews = _others.begin(); childtreeviews != _others.end(); ++childtreeviews )
  {
    if( (*childtreeviews)->Expanded() )
      (*childtreeviews)->Toggle();
  }

  if( mParent )
    if( !mExpanded )
      mParent->Move( -static_cast<int>( ( _others.size() + mFiles.size() ) ), shared_from_this() );
    else
      mParent->Move( ( _others.size() + mFiles.size() ), shared_from_this() );
}

void UITreeView::render() const
{
  if( hidden() )
    return;

  glPushMatrix();
  glTranslatef( x(), y(), 0 );
  mMyButton->render();
  mMyText->render();

  if( mExpanded )
  {
    Others::const_iterator childtreeviews;
    for( childtreeviews = _others.begin(); childtreeviews != _others.end(); ++childtreeviews )
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

UIFrame::Ptr UITreeView::processLeftClick( float mx, float my )
{
  if( hidden() )
    return NULL;

  mx -= x();
  my -= y();

  if( !mMyButton->hidden() && mMyButton->IsHit( mx, my ) )
    return mMyButton->processLeftClick( mx - mMyButton->x(), my - mMyButton->y() );

  std::vector<UIText*>::iterator childfiles;
  if( mSelectFunction )
  {
    for( childfiles = mFiles.begin(); childfiles != mFiles.end(); ++childfiles )
    {
      if( !(*childfiles)->hidden() && (*childfiles)->IsHit( mx, my ) )
      {
        std::string lPath;
        UITreeView::Ptr lParent = shared_from_this();
        while( lParent )
        {
          lPath.insert( 0, std::string( lParent->GetDirectoryName() + "/" ) );
          lParent = lParent->GetParent();
        }
        mSelectFunction( lPath + (*childfiles)->getText() );
        return *childfiles;
      }
    }
  }

  if( mExpanded )
  {
    Others::iterator childtreeviews;
    for( childtreeviews = _others.begin(); childtreeviews != _others.end(); ++childtreeviews )
    {
      (*childtreeviews)->processLeftClick(mx,my);
    }
  }

  return 0;
}
