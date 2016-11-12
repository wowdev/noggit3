#ifndef __UIOBJECTEDITOR_H
#define __UIOBJECTEDITOR_H

#include "UIWindow.h"
#include "Selection.h"
#include "Vec3D.h"

class UIModelImport;
class UIStatusBar;
class UIMapViewGUI;

enum ModelPasteMode
{
  PASTE_ON_TERRAIN,
  PASTE_ON_SELECTION,
  PASTE_ON_MODEL,
  PASTE_ON_CAMERA,
  PASTE_MODE_COUNT
};

class UIObjectEditor : public UIWindow
{
public:
  UIObjectEditor(float x, float y, UIMapViewGUI* mainGui);

  void copy(nameEntry entry);
  void pasteObject();
  void togglePasteMode();

  UIModelImport *modelImport;
  UIStatusBar *filename;
private:
  UIToggleGroup *pasteModeGroup;

  nameEntry selected;
  void setModelName(const std::string &name);
  int pasteMode;
};


#endif
