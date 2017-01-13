// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

#include "Environment.h"
#include "Misc.h"
#include "ModelInstance.h"
#include "application.h" // fonts
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

void SaveObjecttoTXT(UIFrame* f, int)
{
  if (!gWorld->HasSelection())
    return;
  std::string path;

  if (gWorld->IsSelection(eEntry_WMO))
  {
    path = boost::get<selected_wmo_type> (*gWorld->GetCurrentSelection())->wmo->filename();
  }
  else if (gWorld->IsSelection(eEntry_Model))
  {
    path = boost::get<selected_model_type> (*gWorld->GetCurrentSelection())->model->_filename;
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

void toggleMoveModelToCursorPos(bool b, int)
{
  Environment::getInstance()->moveModelToCursorPos = b;
}

void updateHiddenModelsVisibility(UIFrame*, int)
{
  Environment::getInstance()->showModelFromHiddenList = !Environment::getInstance()->showModelFromHiddenList;
}


void clearHiddenModels(UIFrame*, int)
{
  gWorld->clearHiddenModelList();
}

UIObjectEditor::UIObjectEditor(float x, float y, UIMapViewGUI* mainGui)
   : UIWindow(x, y, 270.0f, 320.0f)
   , selected()
   , pasteMode(PASTE_ON_TERRAIN)
{
  filename = new UIStatusBar(0.0f, (float)video.yres() - 60.0f, (float)video.xres(), 30.0f);
  filename->hide();
  mainGui->addChild(filename);

  addChild(new UIText(120.0f, 2.0f, "Object edit", app.getArial14(), eJustifyCenter));
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

  UICheckBox* copyCB = new UICheckBox(5.0f, 110.0f, "Copy model rotation / scale / tilt", toggleCopyModelStats, 0);
  copyCB->setState(Settings::getInstance()->copyModelStats);
  addChild(copyCB);

  addChild(new UIText(5.0f, 137.5f, "Paste Mode:", app.getArial14(), eJustifyLeft));

  pasteModeGroup = new UIToggleGroup(&pasteMode);

  addChild(new UICheckBox(5.0f, 155.0f, "Terrain", pasteModeGroup, PASTE_ON_TERRAIN));
  addChild(new UICheckBox(105.0f, 155.0f, "Selection", pasteModeGroup, PASTE_ON_SELECTION));
  addChild(new UICheckBox(5.0f, 180.0f, "Model", pasteModeGroup, PASTE_ON_MODEL));
  addChild(new UICheckBox(105.0f, 180.0f, "Camera", pasteModeGroup, PASTE_ON_CAMERA));

  pasteModeGroup->Activate(pasteMode);

  UICheckBox* moveToCursorCB = new UICheckBox(5.0f, 215.0f, "Model movement mode: to cursor pos", toggleMoveModelToCursorPos, 0);
  moveToCursorCB->setState(Environment::getInstance()->moveModelToCursorPos);
  addChild(moveToCursorCB);

  addChild(new UIText(190.0f, 250.0f, "Import:", app.getArial14(), eJustifyLeft));
  addChild(new UIButton(190.0f, 270.0f, 75.0f, 30.0f, "To txt", "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp", "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp", SaveObjecttoTXT, 0));
  addChild(new UIButton(190.0f, 295.0f, 75.0f, 30.0f, "From txt", "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp", "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp", showImportModels, 0));

  addChild(new UIButton(5.0f, 245.0f, 150.0f, 30.0f, "Rotation editor", "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp", "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp", toggleRotationEditor, 0));

  addChild(new UIButton(5.0f, 270.0f, 150.0f, 30.0f, "Toggle visibility", "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp", "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp", updateHiddenModelsVisibility, 0));
  addChild(new UIButton(5.0f, 295.0f, 150.0f, 30.0f, "Clear list", "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp", "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp", clearHiddenModels, 0));
}

void UIObjectEditor::pasteObject()
{
  if (selected.which() != eEntry_Model && selected.which() != eEntry_WMO)
  {
    return;
  }

  // default value
  math::vector_3d pos = Environment::getInstance()->get_cursor_pos();

  switch (pasteMode)
  {
    case PASTE_ON_TERRAIN: // use cursor pos
      break;
    case PASTE_ON_MODEL:
      pos = gWorld->getCursorPosOnModel();
      break;
    case PASTE_ON_SELECTION:
      if (gWorld->HasSelection())
      {
        auto selection = *gWorld->GetCurrentSelection();
        if (selection.which() == eEntry_Model)
        {
          pos = boost::get<selected_model_type> (selection)->pos;
        }
        else if (selection.which() == eEntry_WMO)
        {
          pos = boost::get<selected_wmo_type> (selection)->pos;
        }
      } // else: use cursor pos
      break;
    case PASTE_ON_CAMERA:
      pos = gWorld->camera;
      break;
    default:
      LogDebug << "UIObjectEditor::pasteObject: Unknown pasteMode " << pasteMode << std::endl;
      break;
  }

  gWorld->addModel(selected, pos, true);
}

void UIObjectEditor::togglePasteMode()
{
  pasteModeGroup->Activate((pasteMode + 1) % PASTE_MODE_COUNT);
}

void UIObjectEditor::copy(selection_type entry)
{
  if (entry.which() == eEntry_Model)
  {
    auto clone = new ModelInstance(boost::get<selected_model_type> (entry)->model->_filename);
    clone->sc = boost::get<selected_model_type> (entry)->sc;
    clone->dir = boost::get<selected_model_type> (entry)->dir;
    clone->ldir = boost::get<selected_model_type> (entry)->ldir;
    selected = clone;
    setModelName (boost::get<selected_model_type> (entry)->model->_filename);
  }
  else if (entry.which() == eEntry_WMO)
  {
    auto clone = new WMOInstance(boost::get<selected_wmo_type> (entry)->wmo->_filename);
    clone->dir = boost::get<selected_wmo_type> (entry)->dir;
    selected = clone;
    setModelName(boost::get<selected_wmo_type> (entry)->wmo->_filename);
  }
  else
  {
    Environment::getInstance()->clear_clipboard();
    return;
  }

  Environment::getInstance()->set_clipboard(selected);
}

void UIObjectEditor::setModelName(const std::string &name)
{
  std::stringstream ss;
  ss << "Model: " << name;
  filename->setLeftInfo(ss.str());
}
