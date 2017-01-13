// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "UIToggleGroup.h"

#include <map>

#include "UICheckBox.h"

UIToggleGroup::UIToggleGroup(int * pTarget)
	: mTarget(pTarget)
{
}

void UIToggleGroup::Add(UICheckBox::Ptr pFrame, int pValue)
{
	mFrames[pValue] = pFrame;
}

void UIToggleGroup::Activate(UICheckBox::Ptr pFrame)
{
	Frames::iterator pFrameIterator;
	for (Frames::iterator it(mFrames.begin()), end(mFrames.end()); it != end
		; ++it)
	{
    if (it->second == pFrame)
    {
      *mTarget = it->first;
      it->second->setState(true);
    }
    else
    {
      it->second->setState(false);
    }
	}
}

void UIToggleGroup::Activate(int pID)
{
	Frames::iterator pFrame = mFrames.find(pID);
	if (pFrame != mFrames.end())
	{
		for (Frames::iterator it(mFrames.begin()), end(mFrames.end()); it != end
			; ++it)
		{
			it->second->setState(false);
		}
    *mTarget = pID;
		pFrame->second->setState(true);
	}
}
