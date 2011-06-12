#include "ToggleGroup.h"

#include <map>

#include "checkboxUI.h"

ToggleGroup::ToggleGroup( int * pTarget ) : mTarget( pTarget )
{
}

void ToggleGroup::Add( checkboxUI * pFrame, int pValue )
{
  mFrames[pValue] = pFrame;
}

void ToggleGroup::Activate( checkboxUI * pFrame )
{
  std::map<int,checkboxUI*>::iterator pFrameIterator;
  for( pFrameIterator = mFrames.begin(); pFrameIterator != mFrames.end(); pFrameIterator++ )
    pFrameIterator->second->setState( false );

  for( pFrameIterator = mFrames.begin(); pFrameIterator != mFrames.end(); pFrameIterator++ )
    if( pFrameIterator->second == pFrame )
    {
      *mTarget = pFrameIterator->first;
      pFrameIterator->second->setState( true );
    }
}
void ToggleGroup::Activate( int pID )
{
  std::map<int,checkboxUI*>::iterator pFrameIterator, pFrame;
  pFrame = mFrames.find( pID );
  if( pFrame != mFrames.end() )
  {
    for( pFrameIterator = mFrames.begin(); pFrameIterator != mFrames.end(); pFrameIterator++ )
      pFrameIterator->second->setState( false );

    pFrame->second->setState( true );
  }
}
