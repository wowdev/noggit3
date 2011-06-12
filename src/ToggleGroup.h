#ifndef __TOGGLEGROUP_H
#define __TOGGLEGROUP_H

#include <map>

class checkboxUI;

class ToggleGroup
{
private:
	int * mTarget;
	std::map<int,checkboxUI*> mFrames;
public:
	explicit ToggleGroup( int * pTarget );

	void Add( checkboxUI * pFrame, int pValue );

	void Activate( checkboxUI * pFrame );
	void Activate( int pID );
};
#endif
