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
  if (editor->hasSelection())
  {
    float v = std::min(180.0f, std::max(-180.0f, (float)std::atof(value.c_str())));
    textBox->value(misc::floatToStr(v, 3));

    editor->rotationVect->x = v;
  }
}

void updateRotationZ(UITextBox::Ptr textBox, const std::string& value)
{
  UIRotationEditor* editor = (static_cast<UIRotationEditor *>(textBox->parent()));
  if (editor->hasSelection())
  {
    float v = std::min(180.0f, std::max(-180.0f, (float)std::atof(value.c_str())));
    textBox->value(misc::floatToStr(v, 3));

    editor->rotationVect->z = v;
  }  
}

void updateRotationY(UITextBox::Ptr textBox, const std::string& value)
{
  UIRotationEditor* editor = (static_cast<UIRotationEditor *>(textBox->parent()));
  if (editor->hasSelection())
  {
    float v = std::min(360.0f, std::max(0.0f, (float)std::atof(value.c_str())));
    textBox->value(misc::floatToStr(v, 3));

    editor->rotationVect->y = v;
  }  
}

void updatePosX(UITextBox::Ptr textBox, const std::string& value)
{
  UIRotationEditor* editor = (static_cast<UIRotationEditor *>(textBox->parent()));
  if (editor->hasSelection())
  {
    float v = std::atof(value.c_str());
    textBox->value(misc::floatToStr(v, 5));

    editor->posVect->x = v;
  }
}

void updatePosZ(UITextBox::Ptr textBox, const std::string& value)
{
  UIRotationEditor* editor = (static_cast<UIRotationEditor *>(textBox->parent()));
  if (editor->hasSelection())
  {
    float v = std::atof(value.c_str());
    textBox->value(misc::floatToStr(v, 5));

    editor->posVect->z = v;
  }
}

void updatePosY(UITextBox::Ptr textBox, const std::string& value)
{
  UIRotationEditor* editor = (static_cast<UIRotationEditor *>(textBox->parent()));
  if (editor->hasSelection())
  {
    float v = std::atof(value.c_str());
    textBox->value(misc::floatToStr(v, 5));

    editor->posVect->y = v;
  }
}

void updateScale(UITextBox::Ptr textBox, const std::string& value)
{
  UIRotationEditor* editor = (static_cast<UIRotationEditor *>(textBox->parent()));
  if (editor->hasSelection() && !editor->isWmo())
  {
    float v = std::atof(value.c_str());
    textBox->value(misc::floatToStr(v, 2));

    *editor->scale = v;
  }
}


UIRotationEditor::UIRotationEditor(float x, float y)
  : UIWindow(x, y, 120.0f, 270.0f),
  rotationVect(nullptr),
  posVect(nullptr),
  scale(nullptr),
  _selection(false),
  _wmo(false)
{ 
  _tbRotationX = new UITextBox(20.0f, 25.0f, 95.0f, 35.0f, app.getArial12(), updateRotationX);
  _tbRotationZ = new UITextBox(20.0f, 55.0f, 95.0f, 35.0f, app.getArial12(), updateRotationZ);
  _tbRotationY = new UITextBox(20.0f, 98.0f, 95.0f, 35.0f, app.getArial12(), updateRotationY);

  _tbPosX = new UITextBox(20.0f, 145.0f, 95.0f, 35.0f, app.getArial12(), updatePosX);
  _tbPosZ = new UITextBox(20.0f, 170.0f, 95.0f, 35.0f, app.getArial12(), updatePosZ);
  _tbPosY = new UITextBox(20.0f, 195.0f, 95.0f, 35.0f, app.getArial12(), updatePosY);

  _tbScale = new UITextBox(20.0f, 245.0f, 95.0f, 35.0f, app.getArial12(), updateScale);

  addChild(new UIText(5.0f, 5.0f, "Tilt:", app.getArial12(), eJustifyLeft));
  addChild(new UIText(5.0f, 27.0f, "X", app.getArial12(), eJustifyLeft));
  addChild(new UIText(5.0f, 57.0f, "Z", app.getArial12(), eJustifyLeft));
  addChild(new UIText(5.0f, 80.0f, "Rotation:", app.getArial12(), eJustifyLeft));

  addChild(new UIText(5.0f, 125.0f, "Pos:", app.getArial12(), eJustifyLeft));
  addChild(new UIText(5.0f, 147.0f, "X", app.getArial12(), eJustifyLeft));
  addChild(new UIText(5.0f, 172.0f, "Z", app.getArial12(), eJustifyLeft));
  addChild(new UIText(5.0f, 197.0f, "H", app.getArial12(), eJustifyLeft));

  _textScale = new UIText(5.0f, 225.0f, "Scale:", app.getArial12(), eJustifyLeft);
  addChild(_textScale);

  addChild(_tbRotationX);
  addChild(_tbRotationZ);
  addChild(_tbRotationY);
  addChild(_tbPosX);
  addChild(_tbPosZ);
  addChild(_tbPosY);
  addChild(_tbScale);
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
    scale = &(entry->data.model->sc);
    _wmo = false;
    _textScale->show();
    _tbScale->show();
  }
  else if (entry->type == eEntry_WMO)
  {
    rotationVect = &(entry->data.wmo->dir);
    posVect = &(entry->data.wmo->pos);
    _wmo = true;
    _textScale->hide();
    _tbScale->hide();
  }
  else
  {
    _wmo = false;
    clearSelect();
    return;
  }

  _selection = true;
  updateValues();
}

void UIRotationEditor::updateValues()
{
  if (_selection)
  {
    _tbRotationX->value(misc::floatToStr(rotationVect->x, 3));
    _tbRotationY->value(misc::floatToStr(rotationVect->y, 3));
    _tbRotationZ->value(misc::floatToStr(rotationVect->z, 3));
    _tbPosX->value(misc::floatToStr(posVect->x, 5));
    _tbPosY->value(misc::floatToStr(posVect->y, 5));
    _tbPosZ->value(misc::floatToStr(posVect->z, 5));

    if (!_wmo)
    {
      _tbScale->value(misc::floatToStr(*scale));
    }
  }
}

void UIRotationEditor::clearSelect()
{
  _selection = false;
  rotationVect = nullptr;
  posVect = nullptr;
  scale = nullptr;
  _tbRotationX->value("");
  _tbRotationY->value("");
  _tbRotationZ->value("");
  _tbPosX->value("");
  _tbPosY->value("");
  _tbPosZ->value("");
  _tbScale->value("");
}

bool UIRotationEditor::hasFocus() const
{
  return !_hidden &&
         ( _tbRotationX->focus() ||
           _tbRotationY->focus() ||
           _tbRotationZ->focus() ||
           _tbPosX->focus() ||
           _tbPosY->focus() ||
           _tbPosZ->focus() ||
           _tbScale->focus()
         );
}