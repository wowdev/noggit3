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
    textBox->value(misc::floatToStr(v, 3));

    editor->rotationVect->x = v;
  }
}

void updateRotationZ(UITextBox::Ptr textBox, const std::string& value)
{
  UIRotationEditor* editor = (static_cast<UIRotationEditor *>(textBox->parent()));
  if (editor->getSelection())
  {
    float v = std::min(180.0f, std::max(-180.0f, (float)std::atof(value.c_str())));
    textBox->value(misc::floatToStr(v, 3));

    editor->rotationVect->z = v;
  }  
}

void updateRotationY(UITextBox::Ptr textBox, const std::string& value)
{
  UIRotationEditor* editor = (static_cast<UIRotationEditor *>(textBox->parent()));
  if (editor->getSelection())
  {
    float v = std::min(360.0f, std::max(0.0f, (float)std::atof(value.c_str())));
    textBox->value(misc::floatToStr(v, 3));

    editor->rotationVect->y = v;
  }  
}

void updatePosX(UITextBox::Ptr textBox, const std::string& value)
{
  UIRotationEditor* editor = (static_cast<UIRotationEditor *>(textBox->parent()));
  if (editor->getSelection())
  {
    float v = std::atof(value.c_str());
    textBox->value(misc::floatToStr(v, 5));

    editor->posVect->x = v;
  }
}

void updatePosZ(UITextBox::Ptr textBox, const std::string& value)
{
  UIRotationEditor* editor = (static_cast<UIRotationEditor *>(textBox->parent()));
  if (editor->getSelection())
  {
    float v = std::atof(value.c_str());
    textBox->value(misc::floatToStr(v, 5));

    editor->posVect->z = v;
  }
}

void updatePosY(UITextBox::Ptr textBox, const std::string& value)
{
  UIRotationEditor* editor = (static_cast<UIRotationEditor *>(textBox->parent()));
  if (editor->getSelection())
  {
    float v = std::atof(value.c_str());
    textBox->value(misc::floatToStr(v, 5));

    editor->posVect->y = v;
  }
}


UIRotationEditor::UIRotationEditor(float x, float y)
   : UIWindow(x, y, 120.0f, 225.0f),
   rotationVect(nullptr),
   posVect(nullptr),
   selection(false)
{ 
  tbRotationX = new UITextBox(20.0f, 25.0f, 95.0f, 35.0f, app.getArial12(), updateRotationX);
  tbRotationZ = new UITextBox(20.0f, 55.0f, 95.0f, 35.0f, app.getArial12(), updateRotationZ);
  tbRotationY = new UITextBox(20.0f, 98.0f, 95.0f, 35.0f, app.getArial12(), updateRotationY);

  tbPosX = new UITextBox(20.0f, 145.0f, 95.0f, 35.0f, app.getArial12(), updatePosX);
  tbPosZ = new UITextBox(20.0f, 170.0f, 95.0f, 35.0f, app.getArial12(), updatePosZ);
  tbPosY = new UITextBox(20.0f, 195.0f, 95.0f, 35.0f, app.getArial12(), updatePosY);

  addChild(new UIText(5.0f, 5.0f, "Tilt:", app.getArial12(), eJustifyLeft));
  addChild(new UIText(5.0f, 27.0f, "X", app.getArial12(), eJustifyLeft));
  addChild(new UIText(5.0f, 57.0f, "Z", app.getArial12(), eJustifyLeft));
  addChild(new UIText(5.0f, 80.0f, "Rotation:", app.getArial12(), eJustifyLeft));

  addChild(new UIText(5.0f, 125.0f, "Pos:", app.getArial12(), eJustifyLeft));
  addChild(new UIText(5.0f, 147.0f, "X", app.getArial12(), eJustifyLeft));
  addChild(new UIText(5.0f, 172.0f, "Z", app.getArial12(), eJustifyLeft));
  addChild(new UIText(5.0f, 197.0f, "H", app.getArial12(), eJustifyLeft));

  addChild(tbRotationX);
  addChild(tbRotationZ);
  addChild(tbRotationY);
  addChild(tbPosX);
  addChild(tbPosZ);
  addChild(tbPosY);
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
    posVect = &(entry->data.model->pos);
  }
  else if (entry->type == eEntry_WMO)
  {
    rotationVect = &(entry->data.wmo->dir);
    posVect = &(entry->data.wmo->pos);
  }
  else
  {
    clearSelect();
    return;
  }

  selection = true;
  updateValues();
}

void UIRotationEditor::updateValues()
{
  if (selection)
  {
    tbRotationX->value(misc::floatToStr(rotationVect->x, 3));
    tbRotationY->value(misc::floatToStr(rotationVect->y, 3));
    tbRotationZ->value(misc::floatToStr(rotationVect->z, 3));
    tbPosX->value(misc::floatToStr(posVect->x, 5));
    tbPosY->value(misc::floatToStr(posVect->y, 5));
    tbPosZ->value(misc::floatToStr(posVect->z, 5));
  }
}

void UIRotationEditor::clearSelect()
{
  selection = false;
  rotationVect = nullptr;
  tbRotationX->value("");
  tbRotationY->value("");
  tbRotationZ->value("");
  tbPosX->value("");
  tbPosY->value("");
  tbPosZ->value("");
}
