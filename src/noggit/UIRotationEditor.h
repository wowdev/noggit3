#ifndef __UIROTATIONEDITOR_H
#define __UIROTATIONEDITOR_H

#include "UIWindow.h"
#include "Selection.h"

class UIText;
class UITextBox;
class WMOInstance;

class UIRotationEditor : public UIWindow
{
public:
  UIRotationEditor(float x, float y);

  void select(selection_type entry);
  void clearSelect();
  void updateValues();
  void toggle();
  bool hasSelection() const { return _selection; }
  
  bool isWmo() const { return _wmo; }
  void updateWMO();

  // check if any checkbox is focused
  bool hasFocus() const;
  
  math::vector_3d* rotationVect;
  math::vector_3d* posVect;
  float* scale;
private:
  bool _selection;
  bool _wmo;
  WMOInstance* _wmoInstance;

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
