// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/DetailInfos.h>

#include <string>

#include <noggit/application.h> // app.getArial14()
#include <noggit/ui/MapViewGUI.h>
#include <noggit/ui/MinimizeButton.h>
#include <noggit/ui/Text.h>

UIDetailInfos::UIDetailInfos(float xPos, float yPos, float w, float h, UIMapViewGUI *setGui)
	: UIWindow(xPos, yPos, w, h)
	, mainGui(setGui)
	, theInfos(new UIText(8.0f, 7.0f, "", app.getArial14(), eJustifyLeft))
{
	addChild(new UIMinimizeButton(width()));
	addChild(theInfos);
}

void UIDetailInfos::setText(const std::string& t)
{
	theInfos->setText(t);
}
