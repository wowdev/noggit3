#ifndef __UIROTATIONEDITOR_H
#define __UIROTATIONEDITOR_H

#include "UIWindow.h"

class UIText;
class UITextBox;
class nameEntry;

class UIRotationEditor : public UIWindow
{
public:
  UIRotationEditor(float x, float y);

  void select(nameEntry* entry);
  void clearSelect();
  void updateValues();
  void toggle();
  bool hasSelection() const { return _selection; }
  bool isWmo() const { return _wmo; }
  
  Vec3D* rotationVect;
  Vec3D* posVect;
  float* scale;
private:
  bool _selection;
  bool _wmo;

  UITextBox* _tbRotationX;
  UITextBox* _tbRotationY;
  UITextBox* _tbRotationZ;

  UITextBox* _tbPosX;
  UITextBox* _tbPosY;
  UITextBox* _tbPosZ;

  UITextBox* _tbScale;
  UIText* _textScale;
};

#endif
