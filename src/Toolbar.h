#ifndef __TOOLBAR_H
#define __TOOLBAR_H

#include "window.h"
#include "FreeType.h" // fonts.
#include "UIEventClasses.h"

class Gui;
class textUI;
class ToolbarIcon;
class textureUI;

class Toolbar : public window, public UIEventListener
{
private:
	Gui *mainGui;
	void SetIcon( int pIcon, const std::string& pIconFile );
public:
	ToolbarIcon *mToolbarIcons[10];
	textUI	*text;
	// current selected Icon
	int selectedIcon;

	// current active texture
	textureUI *current_texture;
	Toolbar(float x, float y, float width, float height, Gui *setGui);
	void IconSelect(int i);
};
#endif
  