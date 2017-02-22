// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/World.h>
#include <noggit/ui/Button.h>
#include <noggit/ui/Texture.h>
#include <noggit/ui/TextureSwitcher.h>
#include <noggit/ui/TexturingGUI.h>

UITextureSwitcher::UITextureSwitcher (float x_right, float y, UIWindow* parent, const math::vector_3d* camera_pos)
  : UICloseWindow (x_right - 130, y, 130, 200, "swapper", true, [parent] { parent->show(); })
{
  float const textureSize = 110.f;

  addChild (_textureFrom = new UITexture(10, 60, textureSize, textureSize, "tileset\\generic\\black.blp"));

  addChild ( new UIButton ( 10
                          , 65 + textureSize
                          , textureSize
                          , 30
                          , "set destination"
                          , "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp"
                          , "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp"
                          , [this]
                            {
                              if (!!UITexturingGUI::getSelectedTexture())
                              {
                                _textureFrom->setTexture (*UITexturingGUI::getSelectedTexture());
                              }
                            }
                          )
           );

  addChild ( new UIButton ( 10
                          , 30
                          , textureSize
                          , 30
                          , "swap ADT"
                          , "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp"
                          , "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp"
                          , [this, camera_pos]
                            {
                              gWorld->swapTexture (*camera_pos, current_texture());
                            }
                          )
           );
}
