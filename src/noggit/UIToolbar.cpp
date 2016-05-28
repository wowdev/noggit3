// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/UIToolbar.h>

#include <string>

#include <noggit/FreeType.h> // fonts.
#include <noggit/Log.h>
#include <noggit/MapView.h> // MapView
#include <noggit/application.h> // arialn13
#include <noggit/UIMapViewGUI.h>
#include <noggit/UIMinimizeButton.h>
#include <noggit/UIText.h>
#include <noggit/UITexture.h>
#include <noggit/UIToolbarIcon.h>

UIToolbar::UIToolbar(float xPos, float yPos, UIMapViewGUI *setGui)
: UIWindow( xPos, yPos, 105.0f, 380.0f, "interface\\tooltips\\ui-tooltip-border.blp" )
, mainGui( setGui )
, text( new UIText( 8, 7, "Raise/Lower", arialn13, eJustifyLeft ) )
, selectedIcon( -1 )
, current_texture( new UITexture( 0, 0, 92.0f, 92.0f, "tileset\\generic\\black.blp" ) )
{
  movable( true );
  addChild( text );
  addChild( new UIMinimizeButton( width() ) );

  // ground edit
  SetIcon( 0, "Interface\\ICONS\\INV_Elemental_Mote_Earth01.blp" );
  // Flat/blur
  SetIcon( 1, "Interface\\ICONS\\INV_Elemental_Mote_Air01.blp" );
  //3D Paint
  SetIcon( 2, "Interface\\ICONS\\INV_Feather_16.blp" );
  //Holes
  SetIcon( 3, "Interface\\ICONS\\INV_Gizmo_HardenedAdamantiteTube.blp" );
  SetIcon( 4, "Interface\\ICONS\\INV_Misc_Map07.blp" );
  SetIcon( 5, "Interface\\ICONS\\INV_Misc_Net_01.blp" );
  SetIcon( 6, "Interface\\ICONS\\INV_Misc_Flower_02.blp" );
  SetIcon( 7, "Interface\\Icons\\INV_Enchant_EssenceAstralLarge.blp" );
  SetIcon( 8, "Interface\\Icons\\Spell_Shaman_ThunderStorm.blp" );
  SetIcon( 9, "Interface\\Icons\\Spell_Shaman_TidalWaves.blp" );

  IconSelect( 0 );

  UIWindow* texture_border = new UIWindow( 5, 280, 95.0f, 95.0f );
  texture_border->addChild( current_texture );
  addChild( texture_border );
}

void UIToolbar::SetIcon( int pIcon, const std::string& pIconFile )
{
  mToolbarIcons[pIcon] = new UIToolbarIcon( ( pIcon % 2 ) * 50.0f + 5.0f, ( pIcon / 2 ) * 50.0f + 30.0f, pIconFile, std::string( "Interface\\BUTTONS\\CheckButtonGlow.blp" ), pIcon, UIEventConstructorArgument(UIToolbarIcon, this, UIToolbar::IconSelect) );
  addChild( mToolbarIcons[pIcon] );
}

void UIToolbar::set_icon_visual( int pIcon )
{
  const char * Names[] = { "Raise / Lower"
                         , "Flatten / Blur"
                         , "3D Paint"
                         , "Holes"
                         , "AreaID Paint"
                         , "Impassible Flag"
                         , "Not used"
                         , "Not used"
                         , "Not used"
                         , "Not used"
                         };
  text->setText( Names[pIcon] );

  for( int j = 0; j < 10; j++ )
    if( mToolbarIcons[j] )
      mToolbarIcons[j]->selected = false;

  if( !mToolbarIcons[pIcon] )
    return;

  selectedIcon = pIcon;
  mToolbarIcons[pIcon]->selected = true;
}

void UIToolbar::IconSelect( int pIcon )
{
  set_icon_visual (pIcon);

  mainGui->theMapview->set_terrain_editing_mode
    (MapView::terrain_editing_modes (pIcon));
}
