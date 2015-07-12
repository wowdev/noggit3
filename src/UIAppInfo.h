#ifndef __APPINFO_H
#define __APPINFO_H

#include <string>

#include "UICloseWindow.h"
#include "UIText.h"

class UIMapViewGUI;

class UIAppInfo : public UICloseWindow
{
public:
	typedef UIAppInfo* Ptr;

private:
	UIMapViewGUI* mainGui;
	UIText::Ptr theInfos;
	std::string mModelToLoad;

public:
	UIAppInfo(float x, float y, float width, float height, UIMapViewGUI* setGui);
	virtual ~UIAppInfo();
	void setText(const std::string& t);
};
#endif
