// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <iostream>
#include <sstream>
#include <string>
#include <boost/bind.hpp>

#include <noggit/DBC.h>
#include <noggit/Log.h>
#include <noggit/Misc.h>
#include <noggit/application.h> // app.getArial14(), arialn13
#include <noggit/World.h>
#include <noggit/ui/Button.h>
#include <noggit/ui/ListView.h>
#include <noggit/ui/MapViewGUI.h>
#include <noggit/ui/ScrollBar.h>
#include <noggit/ui/Text.h> // UIText
#include <noggit/ui/CloseWindow.h> // UICloseWindow
#include <noggit/ui/WaterTypeBrowser.h>
#include <noggit/ui/Water.h>

UIWaterTypeBrowser::UIWaterTypeBrowser(float xPos, float yPos, float w, float h, UIMapViewGUI *setGui)
  : UICloseWindow(xPos, yPos, w, h, "", true)
  , mainGui(setGui)
{
  addChild(new UIText(10.0f, 3.0f, "Select a water type", app.getArial14(), eJustifyLeft));
  buildTypeList();
}


void UIWaterTypeBrowser::buildTypeList()
{
  removeChild(WaterTypeList);
  WaterTypeList = new UIListView(4, 24, width() - 8, height() - 28, 20);
  WaterTypeList->clickable(true);
  addChild(WaterTypeList);




  //LiquidTypeDB::getLiquidName(gWorld->getWaterType(tileX, tileY));

  for (DBCFile::Iterator i = gLiquidTypeDB.begin(); i != gLiquidTypeDB.end(); ++i)
  {

    UIFrame *curFrame = new UIFrame(1, 1, 1, 1);
    std::stringstream ss;
    ss << i->getInt(LiquidTypeDB::ID) << "-" << LiquidTypeDB::getLiquidName(i->getInt(LiquidTypeDB::ID));
    UIButton *tempButton = new UIButton(0.0f,
      0.0f,
      200.0f,
      28.0f,
      ss.str(),
      "Interface\\DialogFrame\\UI-DialogBox-Background-Dark.blp",
      "Interface\\DialogFrame\\UI-DialogBox-Background-Dark.blp",
      [this, i] { mainGui->guiWater->changeWaterType(i->getInt(LiquidTypeDB::ID)); } //steff: kidding me? we talked about this some h before u did this
      );
    tempButton->setLeft();
    curFrame->addChild(tempButton);
    WaterTypeList->addElement(curFrame);
  }

  WaterTypeList->recalcElements(1);
}
