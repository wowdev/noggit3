#ifndef __WATERIDBROWSER_H
#define __WATERIDBROWSER_H

#include <string>

#include "UIButton.h"
#include "UIWindow.h"
#include "UICloseWindow.h"

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

#endif
