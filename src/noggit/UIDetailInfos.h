#ifndef __DETAILINFOS_H
#define __DETAILINFOS_H

#include <string>

#include "UIWindow.h"

class detailInfos;
class UIMapViewGUI;
class UIText;

class UIDetailInfos : public UIWindow
{
private:
	UIMapViewGUI* mainGui;
	UIText* theInfos;

public:
	UIDetailInfos(float x, float y, float width, float height, UIMapViewGUI *setGui);
	void setText(const std::string& t);
};
#endif
