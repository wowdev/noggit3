#include "UIToolbar.h"

#include <string>

#include "Environment.h" // Environment
#include "FreeType.h" // fonts.
#include "Log.h"
#include "MapView.h" // MapView
#include "Noggit.h" // arialn13
#include "UIMapViewGUI.h"
#include "UIMinimizeButton.h"
#include "UIText.h"
#include "UITexture.h"
#include "UIToolbarIcon.h"

UIToolbar::UIToolbar(float xPos, float yPos, float w, float h, UIMapViewGUI *setGui)
: UIWindow( xPos, yPos, w, h, "interface\\tooltips\\ui-tooltip-border.blp" )
, mainGui( setGui )
, text( new UIText( 8, 7, "Raise/Lower", arialn13, eJustifyLeft ) )
, selectedIcon( -1 )
, current_texture( new UITexture( 0, 0, 92.0f, 92.0f, "tileset\\generic\\black.blp" ) )
{
  this->movable = true;
  this->addChild( text );
  this->addChild( static_cast<UIFrame*>( new UIMinimizeButton( w, this ) ) );

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
  this->addChild( texture_border );
  this->height = 380;

}

void UIToolbar::SetIcon( int pIcon, const std::string& pIconFile )
{
  mToolbarIcons[pIcon] = new UIToolbarIcon( ( pIcon % 2 ) * 50.0f + 5.0f, ( pIcon / 2 ) * 50.0f + 30.0f, 45.0f, 45.0f, pIconFile, std::string( "Interface\\BUTTONS\\CheckButtonGlow.blp" ), pIcon, UIEventConstructorArgument(UIToolbarIcon, this, UIToolbar::IconSelect) );
  this->addChild( mToolbarIcons[pIcon] );  
}

// MapView.cpp
void change_settings_window(int oldid, int newid);
extern int terrainMode;

void UIToolbar::IconSelect( int pIcon )
{
  change_settings_window( selectedIcon, pIcon + 1 > 6 ? 0 : pIcon + 1);

  const char * Names[] = { "Raise / Lower", "Flatten / Blur", "3D Paint", "Holes", "AreaID Paint", "Impassible Flag", "Not used", "Not used", "Not used", "Not used" };
  text->setText( Names[pIcon] );
  
  terrainMode = pIcon;

  Environment::getInstance()->view_holelines = ( pIcon == 3 );

  for( int j = 0; j < 10; j++ )
    if( mToolbarIcons[j] )
      mToolbarIcons[j]->selected = false;
  
  if( !mToolbarIcons[pIcon] )
    return;
  
  selectedIcon = pIcon;
  mToolbarIcons[pIcon]->selected = true;
}
