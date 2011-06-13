#ifndef __TOGGLEGROUP_H
#define __TOGGLEGROUP_H

#include <map>

class UICheckBox;

class UIToggleGroup
{
private:
  int* mTarget;
  std::map<int,UICheckBox*> mFrames;

public:
  explicit UIToggleGroup( int * pTarget );

  void Add( UICheckBox * pFrame, int pValue );

  void Activate( UICheckBox * pFrame );
  void Activate( int pID );
};
#endif
