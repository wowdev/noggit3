#ifndef __CURRENTTEXTURE_H
#define __CURRENTTEXTURE_H

#include <string>

#include "UIEventClasses.h"
#include "UIWindow.h"

class UIMapViewGUI;
class UITexture;

class UICurrentTexture : public UIWindow, public UIEventListener
{
private:
	UIMapViewGUI* mainGui;

public:

	// current active texture
	UITexture* current_texture;
	UICurrentTexture(float x, float y, UIMapViewGUI *setGui);
	void IconSelect(int i);
};
#endif

