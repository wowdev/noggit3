#ifndef __UIOBJECTEDITOR_H
#define __UIOBJECTEDITOR_H

#include "UIWindow.h"
#include "UICheckBox.h"
#include "Selection.h"
#include "Vec3D.h"

class UIObjectEditor : public UIWindow
{
public:
  UIObjectEditor(float x, float y);

  void copy(nameEntry entry);
  void pasteObject(Vec3D pos);

private:
  nameEntry selected;
};


#endif
