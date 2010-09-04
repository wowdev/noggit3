#ifndef __TOOLBAR_H
#define __TOOLBAR_H

class Toolbar;

#include "Gui.h"
#include "window.h"
#include "Icon.h"
#include "textUI.h"
#include "textureUI.h"

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
	void SetIcon( int pIcon, std::string pIconFile );
	void IconSelect(int i);
};
#endif
