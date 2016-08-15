#ifndef __UIOBJECTEDITOR_H
#define __UIOBJECTEDITOR_H

#include "UIWindow.h"
#include "Selection.h"
#include "Vec3D.h"

class UIModelImport;
class UIStatusBar;
class UIMapViewGUI;

class UIObjectEditor : public UIWindow
{
public:
  UIObjectEditor(float x, float y, UIMapViewGUI* mainGui);

  void copy(nameEntry entry);
  void pasteObject(Vec3D pos);

  UIModelImport *modelImport;
  UIStatusBar *filename;

private:
  nameEntry selected;

  void setModelName(const std::string &name);
};


#endif
