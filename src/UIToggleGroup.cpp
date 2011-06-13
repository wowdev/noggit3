#include "UIToggleGroup.h"

#include <map>

#include "UICheckBox.h"

UIToggleGroup::UIToggleGroup( int * pTarget )
: mTarget( pTarget )
{
}

void UIToggleGroup::Add( UICheckBox * pFrame, int pValue )
{
  mFrames[pValue] = pFrame;
}

void UIToggleGroup::Activate( UICheckBox * pFrame )
{
  std::map<int, UICheckBox*>::iterator pFrameIterator;
  for( pFrameIterator = mFrames.begin(); pFrameIterator != mFrames.end(); pFrameIterator++ )
    pFrameIterator->second->setState( false );

  for( pFrameIterator = mFrames.begin(); pFrameIterator != mFrames.end(); pFrameIterator++ )
    if( pFrameIterator->second == pFrame )
    {
      *mTarget = pFrameIterator->first;
      pFrameIterator->second->setState( true );
    }
}
void UIToggleGroup::Activate( int pID )
{
  std::map<int, UICheckBox*>::iterator pFrameIterator, pFrame;
  pFrame = mFrames.find( pID );
  if( pFrame != mFrames.end() )
  {
    for( pFrameIterator = mFrames.begin(); pFrameIterator != mFrames.end(); pFrameIterator++ )
      pFrameIterator->second->setState( false );

    pFrame->second->setState( true );
  }
}
