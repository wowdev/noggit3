#ifndef __TOGGLEGROUP_H
#define __TOGGLEGROUP_H

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
#endif
