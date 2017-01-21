// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/CurrentTexture.h>


#include <noggit/Environment.h> // Environment
#include <noggit/FreeType.h> // fonts.
#include <noggit/Log.h>
#include <noggit/MapView.h> // MapView
#include <noggit/application.h> // app.getapp.getArialn13()()
#include <noggit/ui/MapViewGUI.h>
#include <noggit/ui/Texture.h>


UICurrentTexture::UICurrentTexture(float xPos, float yPos, UIMapViewGUI *setGui)
  : UIWindow(xPos, yPos, 95.0f, 115.0f, "interface\\tooltips\\ui-tooltip-border.blp")
  , mainGui(setGui)
  , current_texture(new UITexture(0, 0, 92.0f, 92.0f, "tileset\\generic\\black.blp"))
{
  movable(false);

  addChild(current_texture);

}
