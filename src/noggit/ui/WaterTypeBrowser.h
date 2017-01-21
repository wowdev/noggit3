// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <string>

#include <noggit/ui/Button.h>
#include <noggit/ui/Window.h>
#include <noggit/ui/CloseWindow.h>

class UIMapViewGUI;
class UIListView;

class UIWaterTypeBrowser : public UICloseWindow
{
public:
  typedef UIWaterTypeBrowser* Ptr;

private:
  UIMapViewGUI *mainGui;
  UIListView* WaterTypeList;

public:
  UIWaterTypeBrowser(float xPos, float yPos, float w, float h, UIMapViewGUI *setGui);

  void setWaterTypeID(UIFrame *f, int id);
  void buildTypeList();
};
