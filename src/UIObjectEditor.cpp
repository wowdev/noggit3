#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

#include "Environment.h"
#include "ModelInstance.h"
#include "Noggit.h" // fonts
#include "Settings.h"
#include "UIButton.h"
#include "UICheckBox.h"
#include "UIMapViewGUI.h"
#include "UIModelImport.h"
#include "UIObjectEditor.h"
#include "UIStatusBar.h"
#include "UIText.h"
#include "Video.h" // video
#include "WMOInstance.h" // WMOInstance
#include "World.h"

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
   : UIWindow(x, y, 400.0f, 120.0f)
   , selected()
{
  filename = new UIStatusBar(0.0f, (float)video.yres() - 60.0f, (float)video.xres(), 30.0f);
  filename->hide();
  mainGui->addChild(filename);
  
  addChild(new UIText(190.0f, 2.0f, "Object edit", app.getArial14(), eJustifyCenter));

  addChild(new UICheckBox(5.0f, 15.0f, "Random rotation", toggleRandomRotation, 0));
  addChild(new UICheckBox(5.0f, 40.0f, "Random tilt", toggleRandomTilt, 0));
  addChild(new UICheckBox(5.0f, 65.0f, "Random scale", toggleRandomSize, 0)); 
  addChild(new UICheckBox(5.0f, 90.0f, "Copy model size / scale / tilt", toggleCopyModelStats, 0));
  
  addChild(new UIButton(180.0f, 95.0f, 120.0f, 30.0f, "Spawn on camera", "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp", "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp", pasteOnCamera, 0));

  addChild(new UIButton(315.0f, 70.0f, 75.0f, 30.0f, "To txt", "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp", "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp", SaveObjecttoTXT, 0));
  addChild(new UIButton(315.0f, 95.0f, 75.0f, 30.0f, "From txt", "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp", "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp", showImportModels, 0));
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
    setModelName(entry.data.model->model->_filename);
  }
  else if (entry.type == eEntry_WMO)
  {
    selected = nameEntry(new WMOInstance(entry.data.wmo->wmo->_filename));
    setModelName(entry.data.wmo->wmo->_filename);
  }

}

void UIObjectEditor::setModelName(const std::string &name)
{
  std::stringstream ss;
  ss << "Model: " << name;
  filename->setLeftInfo(ss.str());
}