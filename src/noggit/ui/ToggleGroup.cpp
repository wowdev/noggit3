// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/ToggleGroup.h>

#include <map>

#include <noggit/ui/CheckBox.h>

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
