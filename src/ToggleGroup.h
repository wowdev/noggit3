#ifndef __TOGGLEGROUP_H
#define __TOGGLEGROUP_H

class ToggleGroup;

#include "checkboxUI.h"
#include <map>

class ToggleGroup
{
private:
	int * mTarget;
	std::map<int,checkboxUI*> mFrames;
public:
	ToggleGroup( int * pTarget );

	void Add( checkboxUI * pFrame, int pValue );

	void Activate( checkboxUI * pFrame );
	void Activate( int pID );
};
#endif
