// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "UICurrentTexture.h"


#include "Environment.h" // Environment
#include "FreeType.h" // fonts.
#include "Log.h"
#include "MapView.h" // MapView
#include "application.h" // app.getapp.getArialn13()()
#include "UIMapViewGUI.h"
#include "UITexture.h"


UICurrentTexture::UICurrentTexture(float xPos, float yPos, UIMapViewGUI *setGui)
	: UIWindow(xPos, yPos, 95.0f, 115.0f, "interface\\tooltips\\ui-tooltip-border.blp")
	, mainGui(setGui)
	, current_texture(new UITexture(0, 0, 92.0f, 92.0f, "tileset\\generic\\black.blp"))
{
	movable(false);

	addChild(current_texture);

}
