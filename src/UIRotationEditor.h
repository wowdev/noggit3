#ifndef __UIROTATIONEDITOR_H
#define __UIROTATIONEDITOR_H

#include "UIWindow.h"

class UITextBox;
class nameEntry;

class UIRotationEditor : public UIWindow
{
public:
  UIRotationEditor(float x, float y);

  void select(nameEntry* entry);
  void clearSelect();
  void toggle();
  bool getSelection() const { return selection; }
  
  Vec3D* rotationVect;
  Vec3D* posVect;
private:
  bool selection;

  UITextBox* tbRotationX;
  UITextBox* tbRotationY;
  UITextBox* tbRotationZ;

  UITextBox* tbPosX;
  UITextBox* tbPosY;
  UITextBox* tbPosZ;
};

#endif
