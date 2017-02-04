// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/Toolbar.h>

#include <string>

#include <noggit/Environment.h> // Environment
#include <noggit/FreeType.h> // fonts.
#include <noggit/Log.h>
#include <noggit/MapView.h> // MapView
#include <noggit/application.h> // app.getapp.getArialn13()()
#include <noggit/ui/MapViewGUI.h>
#include <noggit/ui/MinimizeButton.h>
#include <noggit/ui/Text.h>
#include <noggit/ui/Texture.h>
#include <noggit/ui/ToolbarIcon.h>




UIToolbar::UIToolbar(float xPos, float yPos)
  : UIWindow(xPos, yPos + 10.0f, 45.0f, 405, "interface\\tooltips\\ui-tooltip-border.blp")
  , text(new UIText(6, -26, "TEXT", app.getArialn13(), eJustifyLeft))
  , selectedIcon (editing_mode::ground)
{
  movable(false);
  addChild(text);
  //addChild( new UIMinimizeButton( width() ) );

  SetIcon (editing_mode::ground, "Interface\\ICONS\\INV_Elemental_Mote_Earth01.blp");
  SetIcon (editing_mode::flatten_blur, "Interface\\ICONS\\INV_Elemental_Mote_Air01.blp");
  SetIcon (editing_mode::paint, "Interface\\ICONS\\INV_Feather_16.blp");
  SetIcon (editing_mode::holes, "Interface\\ICONS\\INV_Gizmo_HardenedAdamantiteTube.blp");
  SetIcon (editing_mode::areaid, "Interface\\ICONS\\INV_Misc_Map07.blp");
  SetIcon (editing_mode::flags, "Interface\\ICONS\\INV_Misc_Net_01.blp");
  SetIcon (editing_mode::water, "Interface\\ICONS\\INV_Elemental_Primal_Water.blp");
  SetIcon (editing_mode::light, "Interface\\ICONS\\INV_Enchant_ShardBrilliantSmall.blp");
  SetIcon (editing_mode::mccv, "Interface\\ICONS\\Ability_Mage_MissileBarrage.blp");
  SetIcon (editing_mode::object, "Interface\\ICONS\\INV_Crate_04.blp");

  IconSelect (editing_mode::ground);
}

void UIToolbar::SetIcon(editing_mode pIcon, const std::string& pIconFile)
{
  mToolbarIcons.emplace_back (new UIToolbarIcon ( 5.0f
                                                , mToolbarIcons.size() * 40.0f + 5.0f
                                                , pIconFile
                                                , "Interface\\BUTTONS\\CheckButtonGlow.blp"
                                                , [this, pIcon] { IconSelect (pIcon); }
                                                )
                             );
  addChild(mToolbarIcons.back());
}

//! From MapView.cpp
void change_settings_window(editing_mode oldid, editing_mode newid);
extern editing_mode terrainMode;

void UIToolbar::IconSelect(editing_mode pIcon)
{
  change_settings_window(selectedIcon, pIcon);
  terrainMode = pIcon;

  const char * Names[] = { "Raise / Lower", "Flatten / Blur", "3D Paint", "Holes", "AreaID Paint", "Impassible Flag", "Water edit", "Light edit", "Shader editor", "Object editor" };
  text->setText(Names[static_cast<std::size_t> (pIcon)]);

  Environment::getInstance()->view_holelines = (pIcon == editing_mode::holes);

  for (auto& icon : mToolbarIcons)
  {
    icon->selected = false;
  }

  selectedIcon = pIcon;
  mToolbarIcons[static_cast<std::size_t> (pIcon)]->selected = true;
}
