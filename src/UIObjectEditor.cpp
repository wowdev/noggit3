#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

#include "Environment.h"
#include "Misc.h"
#include "ModelInstance.h"
#include "Noggit.h" // fonts
#include "Settings.h"
#include "UIButton.h"
#include "UICheckBox.h"
#include "UIMapViewGUI.h"
#include "UIModelImport.h"
#include "UIObjectEditor.h"
#include "UIRotationEditor.h"
#include "UIStatusBar.h"
#include "UITextBox.h"
#include "UIText.h"
#include "Video.h" // video
#include "WMOInstance.h" // WMOInstance
#include "World.h"

void updateMinRotation(UITextBox::Ptr textBox, const std::string& value)
{
  float v = std::max(0.0f, (float)std::atof(value.c_str()));
  v = std::min(v, Environment::getInstance()->maxRotation);

  Environment::getInstance()->minRotation = v;
  textBox->value(misc::floatToStr(v));
}

void updateMaxRotation(UITextBox::Ptr textBox, const std::string& value)
{
  float v = std::min(360.0f, (float)std::atof(value.c_str()));
  v = std::max(v, Environment::getInstance()->minRotation);

  Environment::getInstance()->maxRotation = v;
  textBox->value(misc::floatToStr(v));
}

void updateMinTilt(UITextBox::Ptr textBox, const std::string& value)
{
  float v = std::max(-180.0f, (float)std::atof(value.c_str()));
  v = std::min(v, Environment::getInstance()->maxTilt);

  Environment::getInstance()->minTilt = v;
  textBox->value(misc::floatToStr(v));
}

void updateMaxTilt(UITextBox::Ptr textBox, const std::string& value)
{
  float v = std::min(180.0f, (float)std::atof(value.c_str()));
  v = std::max(v, Environment::getInstance()->minTilt);

  Environment::getInstance()->maxTilt = v;
  textBox->value(misc::floatToStr(v));
}

void updateMinScale(UITextBox::Ptr textBox, const std::string& value)
{
  float v = std::max(0.01f, (float)std::atof(value.c_str()));
  v = std::min(v, Environment::getInstance()->maxScale);

  Environment::getInstance()->minScale = v;
  textBox->value(misc::floatToStr(v));
}

void updateMaxScale(UITextBox::Ptr textBox, const std::string& value)
{
  float v = std::min(63.0f, (float)std::atof(value.c_str()));
  v = std::max(v, Environment::getInstance()->minScale);

  Environment::getInstance()->maxScale = v;
  textBox->value(misc::floatToStr(v));
}

void toggleRotationEditor(UIFrame* f, int)
{
  (static_cast<UIMapViewGUI *>(f->parent()->parent()))->rotationEditor->toggle();
}


void showImportModels(UIFrame* f, int)
{
  (static_cast<UIObjectEditor *>(f->parent())->modelImport->show());  
}

void pasteOnCamera(UIFrame* f, int)
{
  (static_cast<UIObjectEditor *>(f->parent())->pasteObject(gWorld->camera));
}

void SaveObjecttoTXT(UIFrame* f, int)
{
  if (!gWorld->HasSelection())
    return;
  std::string path;

  if (gWorld->IsSelection(eEntry_WMO))
  {
    path = gWorld->GetCurrentSelection()->data.wmo->wmo->filename();
  }
  else if (gWorld->IsSelection(eEntry_Model))
  {
    path = gWorld->GetCurrentSelection()->data.model->model->_filename;
  }

  std::ofstream stream(Settings::getInstance()->importFile, std::ios_base::app);
  stream << path << std::endl;
  stream.close();

  (static_cast<UIObjectEditor *>(f->parent())->modelImport->builModelList());
}

void toggleRandomRotation(bool b, int)
{
  Settings::getInstance()->random_rotation = b;
}

void toggleRandomTilt(bool b, int)
{
  Settings::getInstance()->random_tilt = b;
}

void toggleRandomSize(bool b, int)
{
  Settings::getInstance()->random_size = b;
}

void toggleCopyModelStats(bool b, int)
{
  Settings::getInstance()->copyModelStats = b;
}


UIObjectEditor::UIObjectEditor(float x, float y, UIMapViewGUI* mainGui)
   : UIWindow(x, y, 400.0f, 140.0f)
   , selected()
{
  filename = new UIStatusBar(0.0f, (float)video.yres() - 60.0f, (float)video.xres(), 30.0f);
  filename->hide();
  mainGui->addChild(filename);

  addChild(new UIText(190.0f, 2.0f, "Object edit", app.getArial14(), eJustifyCenter));
  addChild(new UIText(195.0f, 22.0f, "Min  /  Max", app.getArial12(), eJustifyCenter));

  Environment* env = Environment::getInstance();
  UITextBox* tb;

  tb = new UITextBox(130.0f, 40.0f, 60.0f, 35.0f, app.getArial12(), updateMinRotation);
  tb->value(misc::floatToStr(env->minRotation));
  addChild(tb);
  tb = new UITextBox(130.0f, 65.0f, 60.0f, 35.0f, app.getArial12(), updateMinTilt);
  tb->value(misc::floatToStr(env->minTilt));
  addChild(tb);
  tb = new UITextBox(130.0f, 90.0f, 60.0f, 35.0f, app.getArial12(), updateMinScale);
  tb->value(misc::floatToStr(env->minScale));
  addChild(tb);
  tb = new UITextBox(200.0f, 40.0f, 60.0f, 35.0f, app.getArial12(), updateMaxRotation);
  tb->value(misc::floatToStr(env->maxRotation));
  addChild(tb);
  tb = new UITextBox(200.0f, 65.0f, 60.0f, 35.0f, app.getArial12(), updateMaxTilt);
  tb->value(misc::floatToStr(env->maxTilt));
  addChild(tb);
  tb = new UITextBox(200.0f, 90.0f, 60.0f, 35.0f, app.getArial12(), updateMaxScale);
  tb->value(misc::floatToStr(env->maxScale));
  addChild(tb);

  addChild(new UICheckBox(5.0f, 35.0f, "Random rotation", toggleRandomRotation, 0));
  addChild(new UICheckBox(5.0f, 60.0f, "Random tilt", toggleRandomTilt, 0));
  addChild(new UICheckBox(5.0f, 85.0f, "Random scale", toggleRandomSize, 0)); 

  addChild(new UIButton(290.0f, 40.0f, 100.0f, 30.0f, "Rotation editor", "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp", "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp", toggleRotationEditor, 0));

  UICheckBox* copyCB = new UICheckBox(5.0f, 110.0f, "Copy model rotation / scale / tilt", toggleCopyModelStats, 0);
  copyCB->setState(Settings::getInstance()->copyModelStats);
  addChild(copyCB);
  
  addChild(new UIButton(190.0f, 115.0f, 120.0f, 30.0f, "Spawn on camera", "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp", "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp", pasteOnCamera, 0));

  addChild(new UIButton(315.0f, 90.0f, 75.0f, 30.0f, "To txt", "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp", "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp", SaveObjecttoTXT, 0));
  addChild(new UIButton(315.0f, 115.0f, 75.0f, 30.0f, "From txt", "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp", "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp", showImportModels, 0));
}

void UIObjectEditor::pasteObject(Vec3D pos)
{
  if (selected.type == eEntry_Model || selected.type == eEntry_WMO)
  {
    gWorld->addModel(selected, pos, true);
  }
}

void UIObjectEditor::copy(nameEntry entry)
{
  if (entry.type == eEntry_Model)
  {
    selected = nameEntry(new ModelInstance(entry.data.model->model->_filename));
    selected.data.model->sc = entry.data.model->sc;
    selected.data.model->dir = entry.data.model->dir;
    selected.data.model->ldir = entry.data.model->ldir;
    setModelName(entry.data.model->model->_filename);
  }
  else if (entry.type == eEntry_WMO)
  {
    selected = nameEntry(new WMOInstance(entry.data.wmo->wmo->_filename));
    selected.data.wmo->dir = entry.data.wmo->dir;
    setModelName(entry.data.wmo->wmo->_filename);
  }
  else
  {
    Environment::getInstance()->clear_clipboard();
    return;
  }

  Environment::getInstance()->set_clipboard(&selected);
}

void UIObjectEditor::setModelName(const std::string &name)
{
  std::stringstream ss;
  ss << "Model: " << name;
  filename->setLeftInfo(ss.str());
}