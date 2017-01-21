// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/AppInfo.h>

#include <string>

#include <noggit/Model.h> // Model
#include <noggit/ModelManager.h> // ModelManager
#include <noggit/application.h> // app.getArial14()
#include <noggit/ui/MapViewGUI.h> // UIMapViewGUI
#include <noggit/ui/Model.h> // UIModel
#include <noggit/ui/Text.h> // UIText

UIAppInfo::UIAppInfo(float xPos, float yPos, float w, float h, UIMapViewGUI* setGui)
  : UICloseWindow(xPos, yPos, w, h, "Application Info", true)
  , mainGui(setGui)
  , theInfos(new UIText(8.0f, 20.0f, app.getArial14(), eJustifyLeft))
  , mModelToLoad("World\\AZEROTH\\ELWYNN\\PASSIVEDOODADS\\Trees\\CanopylessTree01.m2")
{
  this->addChild(this->theInfos);

  // UIModel* myTestmodel = new UIModel( 10.0f, 0.0f, w, h );
  //myTestmodel->setModel( mModelToLoad );
  // this->addChild( myTestmodel );
}

void UIAppInfo::setText(const std::string& t)
{
  this->theInfos->setText(t);
}
