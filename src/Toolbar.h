#ifndef __TOOLBAR_H
#define __TOOLBAR_H

#include "window.h"

class Gui;
class textUI;
class Icon;
class textureUI;

class Toolbar:public window
{
private:
	Gui *mainGui;
public:
	Icon *mToolbarIcons[10];
	textUI	*text;
	// current selected Icon
	int selectedIcon;

	// current active texture
	textureUI *current_texture;
	Toolbar(float x, float y, float width, float height, Gui *setGui);
	void SetIcon( int pIcon, const std::string& pIconFile );
	void IconSelect(int i);
};
#endif
