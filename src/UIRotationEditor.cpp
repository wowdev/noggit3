#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

#include "Environment.h"
#include "Misc.h"
#include "ModelInstance.h"
#include "Noggit.h" // fonts
#include "Selection.h"
#include "UIRotationEditor.h"
#include "UITextBox.h"
#include "UIText.h"
#include "Video.h" // video
#include "WMOInstance.h"

void updateRotationX(UITextBox::Ptr textBox, const std::string& value)
{
  UIRotationEditor* editor = (static_cast<UIRotationEditor *>(textBox->parent()));
  if (editor->getSelection())
  {
    float v = std::min(180.0f, std::max(-180.0f, (float)std::atof(value.c_str())));
    textBox->value(misc::floatToStr(v));

    editor->rotationVect->x = v;
  }
}

void updateRotationZ(UITextBox::Ptr textBox, const std::string& value)
{
  UIRotationEditor* editor = (static_cast<UIRotationEditor *>(textBox->parent()));
  if (editor->getSelection())
  {
    float v = std::min(180.0f, std::max(-180.0f, (float)std::atof(value.c_str())));
    textBox->value(misc::floatToStr(v));

    editor->rotationVect->z = v;
  }  
}

void updateRotationY(UITextBox::Ptr textBox, const std::string& value)
{
  UIRotationEditor* editor = (static_cast<UIRotationEditor *>(textBox->parent()));
  if (editor->getSelection())
  {
    float v = std::min(360.0f, std::max(0.0f, (float)std::atof(value.c_str())));
    textBox->value(misc::floatToStr(v));

    editor->rotationVect->y = v;
  }  
}


UIRotationEditor::UIRotationEditor(float x, float y)
   : UIWindow(x, y, 120.0f, 125.0f),
   rotationVect(nullptr),
   selection(false)
{ 
  tbRotationX = new UITextBox(20.0f, 25.0f, 95.0f, 35.0f, app.getArial12(), updateRotationX);
  tbRotationZ = new UITextBox(20.0f, 55.0f, 95.0f, 35.0f, app.getArial12(), updateRotationZ);
  tbRotationY = new UITextBox(20.0f, 98.0f, 95.0f, 35.0f, app.getArial12(), updateRotationY);

  addChild(new UIText(5.0f, 5.0f, "Tilt:", app.getArial12(), eJustifyLeft));
  addChild(new UIText(5.0f, 27.0f, "X", app.getArial12(), eJustifyLeft));
  addChild(new UIText(5.0f, 57.0f, "Z", app.getArial12(), eJustifyLeft));
  addChild(new UIText(5.0f, 80.0f, "Rotation:", app.getArial12(), eJustifyLeft));

  addChild(tbRotationX);
  addChild(tbRotationZ);
  addChild(tbRotationY);
}

void UIRotationEditor::toggle()
{
  if (_hidden)
  {
    show();
  }
  else
  {
    hide();
  }
}

void UIRotationEditor::select(nameEntry* entry)
{
  if (entry->type == eEntry_Model)
  {
    rotationVect = &(entry->data.model->dir);
  }
  else if (entry->type == eEntry_WMO)
  {
    rotationVect = &(entry->data.wmo->dir);
  }
  else
  {
    clearSelect();
    return;
  }

  selection = true;
  tbRotationX->value(misc::floatToStr(rotationVect->x, 3));
  tbRotationY->value(misc::floatToStr(rotationVect->y, 3));
  tbRotationZ->value(misc::floatToStr(rotationVect->z, 3));
}

void UIRotationEditor::clearSelect()
{
  selection = false;
  rotationVect = nullptr;
  tbRotationX->value("");
  tbRotationY->value("");
  tbRotationZ->value("");
}
