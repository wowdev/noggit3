#include "UIToggleGroup.h"

#include <map>

#include "UICheckBox.h"

UIToggleGroup::UIToggleGroup( int * pTarget )
: mTarget( pTarget )
{
}

void UIToggleGroup::Add( UICheckBox::Ptr pFrame, int pValue )
{
  mFrames[pValue] = pFrame;
}

void UIToggleGroup::Activate( UICheckBox::Ptr pFrame )
{
  Frames::iterator pFrameIterator;
  for( Frames::iterator it( mFrames.begin() ), end( mFrames.end() ); it != end
     ; ++it )
  {
    it->second->setState( false );
  }

  for( Frames::iterator it( mFrames.begin() ), end( mFrames.end() ); it != end
     ; ++it )
  {
    if( it->second == pFrame )
    {
      *mTarget = it->first;
      it->second->setState( true );
    }
  }
}

void UIToggleGroup::Activate( int pID )
{
  Frames::iterator pFrame = mFrames.find( pID );
  if( pFrame != mFrames.end() )
  {
    for( Frames::iterator it( mFrames.begin() ), end( mFrames.end() ); it != end
       ; ++it )
    {
      it->second->setState( false );
    }

    pFrame->second->setState( true );
  }
}
