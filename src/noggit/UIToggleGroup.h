// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <map>

//! \note There is a circular include otherwise. I wish I could use ::Ptr.
class UICheckBox;

class UIToggleGroup
{
public:
	typedef UIToggleGroup* Ptr;

private:
	int* mTarget;
	typedef std::map<int, UICheckBox*> Frames;
	Frames mFrames;

public:
	explicit UIToggleGroup(int * pTarget);

	void Add(UICheckBox* pFrame, int pValue);

	void Activate(UICheckBox* pFrame);
	void Activate(int pID);
};
